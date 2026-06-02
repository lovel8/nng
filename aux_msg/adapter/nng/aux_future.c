/*
 * aux_future.c - 异步 Future 领域实现
 *
 * Copyright (c) 2025 - Licensed under MIT
 */

#include "aux_future.h"

/* ================================================================
 * 生命周期
 * ================================================================ */

aux_err
aux_future_alloc(aux_future *fut, aux_future_callback cb, void *arg)
{
    if (!fut) return AUX_ERR_INVALID_ARG;
    return (aux_err)nng_aio_alloc(&fut->inner, (void (*)(void *))cb, arg);
}

void
aux_future_free(aux_future *fut)
{
    if (fut && fut->inner) {
        nng_aio_free(fut->inner);
        fut->inner = NULL;
    }
}

void
aux_future_reap(aux_future *fut)
{
    if (fut && fut->inner) {
        nng_aio_reap(fut->inner);
        fut->inner = NULL;
    }
}

void
aux_future_stop(aux_future *fut)
{
    if (fut && fut->inner) nng_aio_stop(fut->inner);
}

/* ================================================================
 * 状态查询
 * ================================================================ */

aux_err aux_future_result(aux_future *fut)
    { return (aux_err)nng_aio_result(fut->inner); }

size_t aux_future_count(aux_future *fut)
    { return nng_aio_count(fut->inner); }

bool aux_future_is_busy(aux_future *fut)
    { return nng_aio_busy(fut->inner); }

/* ================================================================
 * 控制操作
 * ================================================================ */

void aux_future_cancel(aux_future *fut)
    { nng_aio_cancel(fut->inner); }

void aux_future_abort(aux_future *fut, aux_err reason)
    { nng_aio_abort(fut->inner, (int)reason); }

void aux_future_wait(aux_future *fut)
    { nng_aio_wait(fut->inner); }

/* ================================================================
 * 超时控制
 * ================================================================ */

void aux_future_set_timeout(aux_future *fut, aux_duration timeout)
    { nng_aio_set_timeout(fut->inner, timeout); }

void aux_future_set_expire(aux_future *fut, aux_timestamp expire)
    { nng_aio_set_expire(fut->inner, expire); }

/* ================================================================
 * 数据传递
 * ================================================================ */

void
aux_future_set_envelope(aux_future *fut, aux_envelope *env)
{
    nng_aio_set_msg(fut->inner, env ? env->inner : NULL);
}

aux_envelope
aux_future_get_envelope(aux_future *fut)
{
    aux_envelope env;
    env.inner = nng_aio_get_msg(fut->inner);
    return env;
}

aux_err aux_future_set_io_vec(aux_future *fut, unsigned n, const aux_io_vec *iov)
    { return (aux_err)nng_aio_set_iov(fut->inner, n, iov); }

aux_err aux_future_set_input(aux_future *fut, unsigned index, void *val)
    { return (aux_err)nng_aio_set_input(fut->inner, index, val); }

void *aux_future_get_input(aux_future *fut, unsigned index)
    { return nng_aio_get_input(fut->inner, index); }

void *aux_future_get_output(aux_future *fut, unsigned index)
    { return nng_aio_get_output(fut->inner, index); }

/* ================================================================
 * 提供者接口
 * ================================================================ */

void aux_future_reset(aux_future *fut)
    { nng_aio_reset(fut->inner); }

bool aux_future_start(aux_future *fut, aux_future_cancel_fn cancel_fn, void *arg)
    { return nng_aio_start(fut->inner, (nng_aio_cancelfn)cancel_fn, arg); }

void aux_future_finish(aux_future *fut, aux_err result)
    { nng_aio_finish(fut->inner, (nng_err)result); }

/* ================================================================
 * 延迟操作
 * ================================================================ */

void aux_future_sleep(aux_duration ms, aux_future *fut)
    { nng_sleep_aio(ms, fut->inner); }
