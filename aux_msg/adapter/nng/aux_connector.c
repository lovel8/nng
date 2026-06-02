/*
 * aux_connector.c - 连接器与接收器领域实现
 *
 * Copyright (c) 2025 - Licensed under MIT
 */

#include "aux_connector.h"

/* ================================================================
 * Connector
 * ================================================================ */

aux_err aux_connector_create(aux_connector *conn, aux_channel ch, const char *addr)
    { return (aux_err)nng_dialer_create(&conn->inner, ch.inner, addr); }

aux_err aux_connector_create_by_uri(aux_connector *conn, aux_channel ch, const aux_uri *uri)
    { return (aux_err)nng_dialer_create_url(&conn->inner, ch.inner, uri); }

aux_err aux_connector_start(aux_connector conn, int flags)
    { return (aux_err)nng_dialer_start(conn.inner, flags); }

void aux_connector_start_async(aux_connector conn, int flags, aux_future *fut)
    { nng_dialer_start_aio(conn.inner, flags, fut->inner); }

aux_err aux_connector_close(aux_connector conn)
    { return (aux_err)nng_dialer_close(conn.inner); }

int aux_connector_id(aux_connector conn)
    { return nng_dialer_id(conn.inner); }

aux_err aux_connector_get_url(aux_connector conn, const aux_uri **url)
    { return (aux_err)nng_dialer_get_url(conn.inner, url); }

/* ---- Connector 选项 ---- */

aux_err aux_connector_set_bool(aux_connector c, const char *k, bool v)
    { return (aux_err)nng_dialer_set_bool(c.inner, k, v); }
aux_err aux_connector_set_int(aux_connector c, const char *k, int v)
    { return (aux_err)nng_dialer_set_int(c.inner, k, v); }
aux_err aux_connector_set_size(aux_connector c, const char *k, size_t v)
    { return (aux_err)nng_dialer_set_size(c.inner, k, v); }
aux_err aux_connector_set_duration(aux_connector c, const char *k, aux_duration v)
    { return (aux_err)nng_dialer_set_ms(c.inner, k, v); }
aux_err aux_connector_set_string(aux_connector c, const char *k, const char *v)
    { return (aux_err)nng_dialer_set_string(c.inner, k, v); }
aux_err aux_connector_set_addr(aux_connector c, const char *k, const aux_net_addr *a)
    { return (aux_err)nng_dialer_set_addr(c.inner, k, a); }

aux_err aux_connector_get_bool(aux_connector c, const char *k, bool *v)
    { return (aux_err)nng_dialer_get_bool(c.inner, k, v); }
aux_err aux_connector_get_int(aux_connector c, const char *k, int *v)
    { return (aux_err)nng_dialer_get_int(c.inner, k, v); }
aux_err aux_connector_get_size(aux_connector c, const char *k, size_t *v)
    { return (aux_err)nng_dialer_get_size(c.inner, k, v); }
aux_err aux_connector_get_duration(aux_connector c, const char *k, aux_duration *v)
    { return (aux_err)nng_dialer_get_ms(c.inner, k, v); }
aux_err aux_connector_get_string(aux_connector c, const char *k, const char **v)
    { return (aux_err)nng_dialer_get_string(c.inner, k, v); }
aux_err aux_connector_get_addr(aux_connector c, const char *k, aux_net_addr *a)
    { (void)c; (void)k; (void)a; return AUX_ERR_NOT_SUPPORTED; }
// 注：nng_dialer_get_addr 在头文件中声明但 NNG 当前版本未实现

/* ================================================================
 * Acceptor
 * ================================================================ */

aux_err aux_acceptor_create(aux_acceptor *acc, aux_channel ch, const char *addr)
    { return (aux_err)nng_listener_create(&acc->inner, ch.inner, addr); }

aux_err aux_acceptor_create_by_uri(aux_acceptor *acc, aux_channel ch, const aux_uri *uri)
    { return (aux_err)nng_listener_create_url(&acc->inner, ch.inner, uri); }

aux_err aux_acceptor_start(aux_acceptor acc, int flags)
    { return (aux_err)nng_listener_start(acc.inner, flags); }

aux_err aux_acceptor_close(aux_acceptor acc)
    { return (aux_err)nng_listener_close(acc.inner); }

int aux_acceptor_id(aux_acceptor acc)
    { return nng_listener_id(acc.inner); }

aux_err aux_acceptor_get_url(aux_acceptor acc, const aux_uri **url)
    { return (aux_err)nng_listener_get_url(acc.inner, url); }

/* ---- Acceptor 选项 ---- */

aux_err aux_acceptor_set_bool(aux_acceptor a, const char *k, bool v)
    { return (aux_err)nng_listener_set_bool(a.inner, k, v); }
aux_err aux_acceptor_set_int(aux_acceptor a, const char *k, int v)
    { return (aux_err)nng_listener_set_int(a.inner, k, v); }
aux_err aux_acceptor_set_size(aux_acceptor a, const char *k, size_t v)
    { return (aux_err)nng_listener_set_size(a.inner, k, v); }
aux_err aux_acceptor_set_duration(aux_acceptor a, const char *k, aux_duration v)
    { return (aux_err)nng_listener_set_ms(a.inner, k, v); }
aux_err aux_acceptor_set_string(aux_acceptor a, const char *k, const char *v)
    { return (aux_err)nng_listener_set_string(a.inner, k, v); }

aux_err aux_acceptor_get_bool(aux_acceptor a, const char *k, bool *v)
    { return (aux_err)nng_listener_get_bool(a.inner, k, v); }
aux_err aux_acceptor_get_int(aux_acceptor a, const char *k, int *v)
    { return (aux_err)nng_listener_get_int(a.inner, k, v); }
aux_err aux_acceptor_get_size(aux_acceptor a, const char *k, size_t *v)
    { return (aux_err)nng_listener_get_size(a.inner, k, v); }
aux_err aux_acceptor_get_duration(aux_acceptor a, const char *k, aux_duration *v)
    { return (aux_err)nng_listener_get_ms(a.inner, k, v); }
aux_err aux_acceptor_get_string(aux_acceptor a, const char *k, const char **v)
    { return (aux_err)nng_listener_get_string(a.inner, k, v); }
