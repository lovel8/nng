/*
 * aux_connection.c - 连接实例领域实现
 *
 * Copyright (c) 2025 - Licensed under MIT
 */

#include "aux_connection.h"

aux_err aux_connection_close(aux_connection conn)
    { return (aux_err)nng_pipe_close(conn.inner); }

int aux_connection_id(aux_connection conn)
    { return nng_pipe_id(conn.inner); }

aux_channel aux_connection_get_channel(aux_connection conn)
    { aux_channel ch; ch.inner = nng_pipe_socket(conn.inner); return ch; }

aux_connector aux_connection_get_connector(aux_connection conn)
    { aux_connector c; c.inner = nng_pipe_dialer(conn.inner); return c; }

aux_acceptor aux_connection_get_acceptor(aux_connection conn)
    { aux_acceptor a; a.inner = nng_pipe_listener(conn.inner); return a; }

aux_err aux_connection_peer_addr(aux_connection conn, aux_net_addr *addr)
    { return (aux_err)nng_pipe_peer_addr(conn.inner, addr); }

aux_err aux_connection_self_addr(aux_connection conn, aux_net_addr *addr)
    { return (aux_err)nng_pipe_self_addr(conn.inner, addr); }

aux_err aux_connection_get_bool(aux_connection conn, const char *key, bool *val)
    { return (aux_err)nng_pipe_get_bool(conn.inner, key, val); }

aux_err aux_connection_get_int(aux_connection conn, const char *key, int *val)
    { return (aux_err)nng_pipe_get_int(conn.inner, key, val); }

aux_err aux_connection_get_size(aux_connection conn, const char *key, size_t *val)
    { return (aux_err)nng_pipe_get_size(conn.inner, key, val); }

aux_err aux_connection_get_duration(aux_connection conn, const char *key, aux_duration *val)
    { return (aux_err)nng_pipe_get_ms(conn.inner, key, val); }

aux_err aux_connection_get_string(aux_connection conn, const char *key, const char **val)
    { return (aux_err)nng_pipe_get_string(conn.inner, key, val); }
