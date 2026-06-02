/*
 * aux_session.c - 会话领域实现
 *
 * Copyright (c) 2025 - Licensed under MIT
 */

#include "aux_session.h"

aux_err aux_session_open(aux_session *sess, aux_channel ch)
    { return (aux_err)nng_ctx_open(&sess->inner, ch.inner); }

aux_err aux_session_close(aux_session sess)
    { return (aux_err)nng_ctx_close(sess.inner); }

int aux_session_id(aux_session sess)
    { return nng_ctx_id(sess.inner); }

void aux_session_send_async(aux_session sess, aux_future *fut)
    { nng_ctx_send(sess.inner, fut->inner); }

void aux_session_recv_async(aux_session sess, aux_future *fut)
    { nng_ctx_recv(sess.inner, fut->inner); }

aux_err aux_session_send_envelope(aux_session sess, aux_envelope *env, int flags)
    { return (aux_err)nng_ctx_sendmsg(sess.inner, env->inner, flags); }

aux_err aux_session_recv_envelope(aux_session sess, aux_envelope *env, int flags)
    { return (aux_err)nng_ctx_recvmsg(sess.inner, &env->inner, flags); }

aux_err aux_session_set_bool(aux_session s, const char *k, bool v)
    { return (aux_err)nng_ctx_set_bool(s.inner, k, v); }
aux_err aux_session_set_int(aux_session s, const char *k, int v)
    { return (aux_err)nng_ctx_set_int(s.inner, k, v); }
aux_err aux_session_set_size(aux_session s, const char *k, size_t v)
    { return (aux_err)nng_ctx_set_size(s.inner, k, v); }
aux_err aux_session_set_duration(aux_session s, const char *k, aux_duration v)
    { return (aux_err)nng_ctx_set_ms(s.inner, k, v); }

aux_err aux_session_get_bool(aux_session s, const char *k, bool *v)
    { return (aux_err)nng_ctx_get_bool(s.inner, k, v); }
aux_err aux_session_get_int(aux_session s, const char *k, int *v)
    { return (aux_err)nng_ctx_get_int(s.inner, k, v); }
aux_err aux_session_get_size(aux_session s, const char *k, size_t *v)
    { return (aux_err)nng_ctx_get_size(s.inner, k, v); }
aux_err aux_session_get_duration(aux_session s, const char *k, aux_duration *v)
    { return (aux_err)nng_ctx_get_ms(s.inner, k, v); }
