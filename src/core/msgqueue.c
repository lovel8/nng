//
// Copyright 2021 Staysail Systems, Inc. <info@staysail.tech>
// Copyright 2018 Capitar IT Group BV <info@capitar.com>
//
// This software is supplied under the terms of the MIT License, a
// copy of which should be located in the distribution where this
// file was obtained (LICENSE.txt).  A copy of the license may also be
// found online at https://opensource.org/licenses/MIT.
//

#include "msgqueue.h"
#include "aio.h"
#include "defs.h"
#include "list.h"
#include "message.h"
#include "pollable.h"

// Message queue with 2-level priority support.
//
// Two independent ring buffers: hi (priority=1) and lo (priority=0).
// Dequeue always drains hi first. When total capacity is exhausted,
// a high-priority message can evict the oldest lo message.
//
// Total messages = hi_len + lo_len, capped at mq_cap.

struct nni_msgq {
	nni_mtx   mq_lock;
	unsigned  mq_cap;
	unsigned  mq_alloc; // alloc = cap + 2
	bool      mq_closed;

	// High-priority ring
	unsigned  mq_hi_len;
	unsigned  mq_hi_get;
	unsigned  mq_hi_put;
	nni_msg **mq_hi_msgs;

	// Normal-priority ring
	unsigned  mq_lo_len;
	unsigned  mq_lo_get;
	unsigned  mq_lo_put;
	nni_msg **mq_lo_msgs;

	nni_list mq_aio_putq;
	nni_list mq_aio_getq;

	nni_pollable mq_sendable;
	nni_pollable mq_recvable;
};

static void nni_msgq_run_notify(nni_msgq *);

// ---- helpers for the priority ring buffers ----

static inline nni_msg *
mq_ring_pop(nni_msg **ring, unsigned *len, unsigned *get, unsigned alloc)
{
	nni_msg *msg = ring[*get];
	ring[*get] = NULL;
	*get = (*get + 1) % alloc;
	(*len)--;
	return (msg);
}

static inline void
mq_ring_push(
    nni_msg **ring, unsigned *len, unsigned *put, unsigned alloc, nni_msg *msg)
{
	ring[*put] = msg;
	*put = (*put + 1) % alloc;
	(*len)++;
}

static inline unsigned
mq_total(nni_msgq *mq)
{
	return (mq->mq_hi_len + mq->mq_lo_len);
}

// Evict the oldest normal-priority message. Returns true on success.
static bool
mq_try_evict_lo(nni_msgq *mq)
{
	if (mq->mq_lo_len == 0) {
		return (false);
	}
	nni_msg *old = mq_ring_pop(
	    mq->mq_lo_msgs, &mq->mq_lo_len, &mq->mq_lo_get, mq->mq_alloc);
	nni_msg_free(old);
	return (true);
}

int
nni_msgq_init(nni_msgq **mqp, unsigned cap)
{
	struct nni_msgq *mq;
	unsigned         alloc;

	// alloc = cap + 2:
	//   +1: support cap == 0 (unbuffered, like Go channel)
	//   +1: pushback (e.g. REQ retry)
	alloc = cap + 2;

	if ((mq = NNI_ALLOC_STRUCT(mq)) == NULL) {
		return (NNG_ENOMEM);
	}
	if ((mq->mq_hi_msgs = nni_zalloc(sizeof(nng_msg *) * alloc)) == NULL) {
		NNI_FREE_STRUCT(mq);
		return (NNG_ENOMEM);
	}
	if ((mq->mq_lo_msgs = nni_zalloc(sizeof(nng_msg *) * alloc)) == NULL) {
		nni_free(mq->mq_hi_msgs, sizeof(nng_msg *) * alloc);
		NNI_FREE_STRUCT(mq);
		return (NNG_ENOMEM);
	}

	nni_aio_list_init(&mq->mq_aio_putq);
	nni_aio_list_init(&mq->mq_aio_getq);
	nni_mtx_init(&mq->mq_lock);
	nni_pollable_init(&mq->mq_recvable);
	nni_pollable_init(&mq->mq_sendable);

	mq->mq_cap    = cap;
	mq->mq_alloc  = alloc;
	mq->mq_closed = false;
	mq->mq_hi_len = 0;
	mq->mq_hi_get = 0;
	mq->mq_hi_put = 0;
	mq->mq_lo_len = 0;
	mq->mq_lo_get = 0;
	mq->mq_lo_put = 0;
	*mqp          = mq;

	return (0);
}

static void
mq_free_ring(nni_msg **ring, unsigned *len, unsigned *get, unsigned alloc)
{
	while (*len > 0) {
		nni_msg_free(mq_ring_pop(ring, len, get, alloc));
	}
}

void
nni_msgq_fini(nni_msgq *mq)
{
	if (mq == NULL) {
		return;
	}
	nni_mtx_fini(&mq->mq_lock);

	mq_free_ring(mq->mq_hi_msgs, &mq->mq_hi_len, &mq->mq_hi_get,
	    mq->mq_alloc);
	mq_free_ring(mq->mq_lo_msgs, &mq->mq_lo_len, &mq->mq_lo_get,
	    mq->mq_alloc);

	nni_pollable_fini(&mq->mq_sendable);
	nni_pollable_fini(&mq->mq_recvable);

	nni_free(mq->mq_hi_msgs, sizeof(nng_msg *) * mq->mq_alloc);
	nni_free(mq->mq_lo_msgs, sizeof(nng_msg *) * mq->mq_alloc);
	NNI_FREE_STRUCT(mq);
}

// Push a message into the appropriate priority ring.
// Returns true on success (message consumed).
static bool
mq_put_prio(nni_msgq *mq, nni_msg *msg)
{
	unsigned  total = mq_total(mq);
	unsigned *len, *put;
	nni_msg **ring;

	if (nni_msg_get_priority(msg) > 0) {
		ring = mq->mq_hi_msgs;
		len  = &mq->mq_hi_len;
		put  = &mq->mq_hi_put;
	} else {
		ring = mq->mq_lo_msgs;
		len  = &mq->mq_lo_len;
		put  = &mq->mq_lo_put;
	}

	if (total < mq->mq_cap) {
		mq_ring_push(ring, len, put, mq->mq_alloc, msg);
		return (true);
	}

	// Queue full. Only high-priority can evict a lo message.
	if (nni_msg_get_priority(msg) > 0 && mq_try_evict_lo(mq)) {
		mq_ring_push(ring, len, put, mq->mq_alloc, msg);
		return (true);
	}

	return (false);
}

// Pop the next message: high-priority first, then normal.
static nni_msg *
mq_get_prio(nni_msgq *mq)
{
	if (mq->mq_hi_len > 0) {
		return (mq_ring_pop(
		    mq->mq_hi_msgs, &mq->mq_hi_len, &mq->mq_hi_get, mq->mq_alloc));
	}
	if (mq->mq_lo_len > 0) {
		return (mq_ring_pop(
		    mq->mq_lo_msgs, &mq->mq_lo_len, &mq->mq_lo_get, mq->mq_alloc));
	}
	return (NULL);
}

static void
nni_msgq_run_putq(nni_msgq *mq)
{
	nni_aio *waio;

	while ((waio = nni_list_first(&mq->mq_aio_putq)) != NULL) {
		nni_msg *msg = nni_aio_get_msg(waio);
		size_t   len = nni_msg_len(msg);
		nni_aio *raio;

		// If a reader is waiting, deliver directly (unbuffered path).
		if ((raio = nni_list_first(&mq->mq_aio_getq)) != NULL) {
			nni_aio_set_msg(waio, NULL);
			nni_aio_list_remove(waio);
			nni_aio_list_remove(raio);
			nni_aio_finish_msg(raio, msg);
			nni_aio_finish(waio, 0, len);
			continue;
		}

		// Try to buffer it.
		if (mq_put_prio(mq, msg)) {
			nni_list_remove(&mq->mq_aio_putq, waio);
			nni_aio_set_msg(waio, NULL);
			nni_aio_finish(waio, 0, len);
			continue;
		}

		// No progress possible; wait.
		break;
	}
}

static void
nni_msgq_run_getq(nni_msgq *mq)
{
	nni_aio *raio;

	while ((raio = nni_list_first(&mq->mq_aio_getq)) != NULL) {
		nni_aio *waio;

		if (mq_total(mq) != 0) {
			nni_aio_list_remove(raio);
			nni_aio_finish_msg(raio, mq_get_prio(mq));
			continue;
		}

		// Unbuffered path: match with a waiting writer.
		if ((waio = nni_list_first(&mq->mq_aio_putq)) != NULL) {
			nni_msg *msg = nni_aio_get_msg(waio);
			size_t   len = nni_msg_len(msg);

			nni_aio_set_msg(waio, NULL);
			nni_aio_list_remove(waio);
			nni_aio_finish(waio, 0, len);

			nni_aio_list_remove(raio);
			nni_aio_finish_msg(raio, msg);
			continue;
		}

		break;
	}
}

static void
nni_msgq_run_notify(nni_msgq *mq)
{
	unsigned total = mq_total(mq);

	if (total < mq->mq_cap || !nni_list_empty(&mq->mq_aio_getq)) {
		nni_pollable_raise(&mq->mq_sendable);
	} else {
		nni_pollable_clear(&mq->mq_sendable);
	}
	if (total != 0 || !nni_list_empty(&mq->mq_aio_putq)) {
		nni_pollable_raise(&mq->mq_recvable);
	} else {
		nni_pollable_clear(&mq->mq_recvable);
	}
}

static void
nni_msgq_cancel(nni_aio *aio, void *arg, nng_err rv)
{
	nni_msgq *mq = arg;

	nni_mtx_lock(&mq->mq_lock);
	if (nni_aio_list_active(aio)) {
		nni_aio_list_remove(aio);
		nni_aio_finish_error(aio, rv);
	}
	nni_msgq_run_notify(mq);
	nni_mtx_unlock(&mq->mq_lock);
}

void
nni_msgq_aio_put(nni_msgq *mq, nni_aio *aio)
{
	nni_mtx_lock(&mq->mq_lock);
	if (!nni_aio_start(aio, nni_msgq_cancel, mq)) {
		nni_mtx_unlock(&mq->mq_lock);
		return;
	}
	nni_aio_list_append(&mq->mq_aio_putq, aio);
	nni_msgq_run_putq(mq);
	nni_msgq_run_notify(mq);
	nni_mtx_unlock(&mq->mq_lock);
}

void
nni_msgq_aio_get(nni_msgq *mq, nni_aio *aio)
{
	nni_mtx_lock(&mq->mq_lock);
	if (!nni_aio_start(aio, nni_msgq_cancel, mq)) {
		nni_mtx_unlock(&mq->mq_lock);
		return;
	}
	nni_aio_list_append(&mq->mq_aio_getq, aio);
	nni_msgq_run_getq(mq);
	nni_msgq_run_notify(mq);
	nni_mtx_unlock(&mq->mq_lock);
}

int
nni_msgq_tryput(nni_msgq *mq, nni_msg *msg)
{
	nni_aio *raio;

	nni_mtx_lock(&mq->mq_lock);
	if (mq->mq_closed) {
		nni_mtx_unlock(&mq->mq_lock);
		return (NNG_ECLOSED);
	}

	// Deliver directly to a waiting reader.
	if ((raio = nni_list_first(&mq->mq_aio_getq)) != NULL) {
		nni_list_remove(&mq->mq_aio_getq, raio);
		nni_aio_finish_msg(raio, msg);
		nni_msgq_run_notify(mq);
		nni_mtx_unlock(&mq->mq_lock);
		return (0);
	}

	if (mq_put_prio(mq, msg)) {
		nni_msgq_run_notify(mq);
		nni_mtx_unlock(&mq->mq_lock);
		return (0);
	}

	nni_mtx_unlock(&mq->mq_lock);
	return (NNG_EAGAIN);
}

void
nni_msgq_close(nni_msgq *mq)
{
	nni_aio *aio;

	nni_mtx_lock(&mq->mq_lock);
	mq->mq_closed = true;

	mq_free_ring(mq->mq_hi_msgs, &mq->mq_hi_len, &mq->mq_hi_get,
	    mq->mq_alloc);
	mq_free_ring(mq->mq_lo_msgs, &mq->mq_lo_len, &mq->mq_lo_get,
	    mq->mq_alloc);

	while (((aio = nni_list_first(&mq->mq_aio_getq)) != NULL) ||
	    ((aio = nni_list_first(&mq->mq_aio_putq)) != NULL)) {
		nni_aio_list_remove(aio);
		nni_aio_finish_error(aio, NNG_ECLOSED);
	}
	nni_mtx_unlock(&mq->mq_lock);
}

int
nni_msgq_cap(nni_msgq *mq)
{
	int rv;
	nni_mtx_lock(&mq->mq_lock);
	rv = (int) mq->mq_cap;
	nni_mtx_unlock(&mq->mq_lock);
	return (rv);
}

// Copy messages from a ring to a newly allocated ring (used by resize).
static void
mq_copy_ring(nni_msg **src, unsigned *src_len, unsigned *src_get,
    unsigned src_alloc, nni_msg **dst, unsigned *dst_len, unsigned *dst_put,
    unsigned dst_alloc)
{
	while (*src_len > 0) {
		nni_msg *msg = mq_ring_pop(src, src_len, src_get, src_alloc);
		mq_ring_push(dst, dst_len, dst_put, dst_alloc, msg);
	}
}

int
nni_msgq_resize(nni_msgq *mq, int cap)
{
	unsigned  new_alloc;
	unsigned  old_alloc;

	new_alloc = (unsigned) cap + 2;

	nni_mtx_lock(&mq->mq_lock);

	// Drop excess messages: normal-priority first, then high-priority.
	while (mq_total(mq) > ((unsigned) cap + 1)) {
		if (mq->mq_lo_len > 0) {
			nni_msg_free(mq_ring_pop(mq->mq_lo_msgs, &mq->mq_lo_len,
			    &mq->mq_lo_get, mq->mq_alloc));
		} else if (mq->mq_hi_len > 0) {
			nni_msg_free(mq_ring_pop(mq->mq_hi_msgs, &mq->mq_hi_len,
			    &mq->mq_hi_get, mq->mq_alloc));
		} else {
			break;
		}
	}

	if (new_alloc > mq->mq_alloc) {
		// Growing: allocate new larger rings and migrate.
		nni_msg **new_hi, **new_lo;
		unsigned  new_hi_len, new_hi_put;
		unsigned  new_lo_len, new_lo_put;

		new_hi = nni_zalloc(sizeof(nni_msg *) * new_alloc);
		new_lo = nni_zalloc(sizeof(nni_msg *) * new_alloc);
		if (new_hi == NULL || new_lo == NULL) {
			nni_free(new_hi, sizeof(nni_msg *) * new_alloc);
			nni_free(new_lo, sizeof(nni_msg *) * new_alloc);
			nni_mtx_unlock(&mq->mq_lock);
			return (NNG_ENOMEM);
		}

		old_alloc  = mq->mq_alloc;
		new_hi_len = 0;
		new_hi_put = 0;
		new_lo_len = 0;
		new_lo_put = 0;

		mq_copy_ring(mq->mq_hi_msgs, &mq->mq_hi_len, &mq->mq_hi_get,
		    old_alloc, new_hi, &new_hi_len, &new_hi_put, new_alloc);
		mq_copy_ring(mq->mq_lo_msgs, &mq->mq_lo_len, &mq->mq_lo_get,
		    old_alloc, new_lo, &new_lo_len, &new_lo_put, new_alloc);

		nni_free(mq->mq_hi_msgs, sizeof(nni_msg *) * old_alloc);
		nni_free(mq->mq_lo_msgs, sizeof(nni_msg *) * old_alloc);

		mq->mq_hi_msgs = new_hi;
		mq->mq_lo_msgs = new_lo;
		mq->mq_hi_len  = new_hi_len;
		mq->mq_hi_get  = 0;
		mq->mq_hi_put  = new_hi_put;
		mq->mq_lo_len  = new_lo_len;
		mq->mq_lo_get  = 0;
		mq->mq_lo_put  = new_lo_put;
	}

	// Always update cap and alloc (shrinking case).
	mq->mq_cap   = (unsigned) cap;
	mq->mq_alloc = new_alloc;

	nni_mtx_unlock(&mq->mq_lock);
	return (0);
}

int
nni_msgq_get_recvable(nni_msgq *mq, nni_pollable **sp)
{
	nni_mtx_lock(&mq->mq_lock);
	nni_msgq_run_notify(mq);
	nni_mtx_unlock(&mq->mq_lock);
	*sp = &mq->mq_recvable;
	return (0);
}

int
nni_msgq_get_sendable(nni_msgq *mq, nni_pollable **sp)
{
	nni_mtx_lock(&mq->mq_lock);
	nni_msgq_run_notify(mq);
	nni_mtx_unlock(&mq->mq_lock);
	*sp = &mq->mq_sendable;
	return (0);
}
