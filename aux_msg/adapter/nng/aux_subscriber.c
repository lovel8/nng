/*
 * aux_subscriber.c - 订阅者领域实现
 *
 * Copyright (c) 2025 - Licensed under MIT
 */

#include "aux_subscriber.h"

#ifdef NNG_HAVE_SUB0

aux_err aux_subscriber_subscribe(aux_channel ch, const void *pattern, size_t len)
    { return (aux_err)nng_sub0_socket_subscribe(ch.inner, pattern, len); }

aux_err aux_subscriber_unsubscribe(aux_channel ch, const void *pattern, size_t len)
    { return (aux_err)nng_sub0_socket_unsubscribe(ch.inner, pattern, len); }

aux_err aux_session_subscribe(aux_session sess, const void *pattern, size_t len)
    { return (aux_err)nng_sub0_ctx_subscribe(sess.inner, pattern, len); }

aux_err aux_session_unsubscribe(aux_session sess, const void *pattern, size_t len)
    { return (aux_err)nng_sub0_ctx_unsubscribe(sess.inner, pattern, len); }

#endif /* NNG_HAVE_SUB0 */
