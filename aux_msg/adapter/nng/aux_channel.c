/*
 * aux_channel.c - 通信通道（Channel）领域实现
 *
 * 装饰器模式实现：每个 aux_ 函数内部委托给对应的 nng_ 函数。
 * 不修改 NNG 任何源码，仅在调用前后做参数转换和错误码映射。
 *
 * Copyright (c) 2025 - Licensed under MIT
 */

#include "aux_channel.h"
#include "message.h" // nni_msg_set_priority

#include <string.h>

/* ================================================================
 * 内部辅助：角色 → NNG 协议 open 函数映射
 * ================================================================ */

static aux_err
channel_open_inner(aux_channel *ch, aux_channel_role role, bool raw)
{
    int rv = 0;

    switch (role) {
#ifdef NNG_HAVE_PUB0
    case AUX_ROLE_PUBLISHER:
        rv = raw ? nng_pub0_open_raw(&ch->inner) : nng_pub0_open(&ch->inner);
        break;
#endif
#ifdef NNG_HAVE_SUB0
    case AUX_ROLE_SUBSCRIBER:
        rv = raw ? nng_sub0_open_raw(&ch->inner) : nng_sub0_open(&ch->inner);
        break;
#endif
#ifdef NNG_HAVE_REQ0
    case AUX_ROLE_REQUESTER:
        rv = raw ? nng_req0_open_raw(&ch->inner) : nng_req0_open(&ch->inner);
        break;
#endif
#ifdef NNG_HAVE_REP0
    case AUX_ROLE_REPLIER:
        rv = raw ? nng_rep0_open_raw(&ch->inner) : nng_rep0_open(&ch->inner);
        break;
#endif
#ifdef NNG_HAVE_PUSH0
    case AUX_ROLE_PUSHER:
        rv = raw ? nng_push0_open_raw(&ch->inner) : nng_push0_open(&ch->inner);
        break;
#endif
#ifdef NNG_HAVE_PULL0
    case AUX_ROLE_PULLER:
        rv = raw ? nng_pull0_open_raw(&ch->inner) : nng_pull0_open(&ch->inner);
        break;
#endif
#ifdef NNG_HAVE_SURVEYOR0
    case AUX_ROLE_SURVEYOR:
        rv = raw ? nng_surveyor0_open_raw(&ch->inner)
                 : nng_surveyor0_open(&ch->inner);
        break;
#endif
#ifdef NNG_HAVE_RESPONDENT0
    case AUX_ROLE_RESPONDENT:
        rv = raw ? nng_respondent0_open_raw(&ch->inner)
                 : nng_respondent0_open(&ch->inner);
        break;
#endif
#ifdef NNG_HAVE_PAIR0
    case AUX_ROLE_PEER_PAIR0:
        rv = raw ? nng_pair0_open_raw(&ch->inner) : nng_pair0_open(&ch->inner);
        break;
#endif
#ifdef NNG_HAVE_PAIR1
    case AUX_ROLE_PEER_PAIR1:
        rv = raw ? nng_pair1_open_raw(&ch->inner) : nng_pair1_open(&ch->inner);
        break;
#endif
#ifdef NNG_HAVE_BUS0
    case AUX_ROLE_BUS:
        rv = raw ? nng_bus0_open_raw(&ch->inner) : nng_bus0_open(&ch->inner);
        break;
#endif
    default:
        return AUX_ERR_INVALID_ARG;
    }

    return (aux_err)rv;
}

aux_err
aux_channel_open(aux_channel *ch, aux_channel_role role)
{
    if (!ch) return AUX_ERR_INVALID_ARG;
    return channel_open_inner(ch, role, false);
}

aux_err
aux_channel_open_raw(aux_channel *ch, aux_channel_role role)
{
    if (!ch) return AUX_ERR_INVALID_ARG;
    return channel_open_inner(ch, role, true);
}

aux_err
aux_channel_close(aux_channel ch)
{
    return (aux_err)nng_socket_close(ch.inner);
}

int
aux_channel_id(aux_channel ch)
{
    return nng_socket_id(ch.inner);
}

/* ================================================================
 * 通信能力
 * ================================================================ */

aux_err
aux_channel_send(aux_channel ch, const void *data, size_t size, int flags)
{
    return (aux_err)nng_send(ch.inner, data, size, flags);
}

aux_err
aux_channel_recv(aux_channel ch, void *data, size_t *actual_size, int flags)
{
    return (aux_err)nng_recv(ch.inner, data, actual_size, flags);
}

void
aux_channel_send_async(aux_channel ch, aux_future *fut)
{
    nng_socket_send(ch.inner, fut->inner);
}

void
aux_channel_recv_async(aux_channel ch, aux_future *fut)
{
    nng_socket_recv(ch.inner, fut->inner);
}

/* ================================================================
 * 消息信封收发
 * ================================================================ */

aux_err
aux_channel_send_envelope(aux_channel ch, aux_envelope *env, int flags, aux_priority priority)
{
    nni_msg_set_priority(env->inner, (int) priority);
    return (aux_err)nng_sendmsg(ch.inner, env->inner, flags);
}

aux_err
aux_channel_recv_envelope(aux_channel ch, aux_envelope *env, int flags)
{
    return (aux_err)nng_recvmsg(ch.inner, &env->inner, flags);
}

/* ================================================================
 * 连接管理
 * ================================================================ */

aux_err
aux_channel_accept_at(
    aux_channel ch, const char *addr, aux_acceptor *acceptor, int flags)
{
    nng_listener l = NNG_LISTENER_INITIALIZER;
    int rv;

    if (acceptor) {
        rv = nng_listen(ch.inner, addr, &l, flags);
        if (rv == 0) acceptor->inner = l;
    } else {
        rv = nng_listen(ch.inner, addr, NULL, flags);
    }

    return (aux_err)rv;
}

aux_err
aux_channel_connect_to(
    aux_channel ch, const char *addr, aux_connector *connector, int flags)
{
    nng_dialer d = NNG_DIALER_INITIALIZER;
    int rv;

    if (connector) {
        rv = nng_dial(ch.inner, addr, &d, flags);
        if (rv == 0) connector->inner = d;
    } else {
        rv = nng_dial(ch.inner, addr, NULL, flags);
    }

    return (aux_err)rv;
}

aux_err
aux_channel_accept_at_uri(
    aux_channel ch, const aux_uri *uri, aux_acceptor *acceptor, int flags)
{
    nng_listener l = NNG_LISTENER_INITIALIZER;
    int rv;

    if (acceptor) {
        rv = nng_listen_url(ch.inner, uri, &l, flags);
        if (rv == 0) acceptor->inner = l;
    } else {
        rv = nng_listen_url(ch.inner, uri, NULL, flags);
    }

    return (aux_err)rv;
}

aux_err
aux_channel_connect_to_uri(
    aux_channel ch, const aux_uri *uri, aux_connector *connector, int flags)
{
    nng_dialer d = NNG_DIALER_INITIALIZER;
    int rv;

    if (connector) {
        rv = nng_dial_url(ch.inner, uri, &d, flags);
        if (rv == 0) connector->inner = d;
    } else {
        rv = nng_dial_url(ch.inner, uri, NULL, flags);
    }

    return (aux_err)rv;
}

/* ================================================================
 * 连接事件通知
 * ================================================================ */

aux_err
aux_channel_on_connection_event(
    aux_channel ch, aux_conn_event event, aux_conn_callback cb, void *arg)
{
    nng_pipe_ev ev;

    switch (event) {
    case AUX_CONN_EVENT_ADD_PRE:  ev = NNG_PIPE_EV_ADD_PRE;  break;
    case AUX_CONN_EVENT_ADD_POST: ev = NNG_PIPE_EV_ADD_POST; break;
    case AUX_CONN_EVENT_REM_POST: ev = NNG_PIPE_EV_REM_POST; break;
    default: return AUX_ERR_INVALID_ARG;
    }

    return (aux_err)nng_pipe_notify(ch.inner, ev, (nng_pipe_cb)cb, arg);
}

/* ================================================================
 * 选项读写
 * ================================================================ */

aux_err aux_channel_set_bool(aux_channel ch, const char *key, bool val)
    { return (aux_err)nng_socket_set_bool(ch.inner, key, val); }

aux_err aux_channel_set_int(aux_channel ch, const char *key, int val)
    { return (aux_err)nng_socket_set_int(ch.inner, key, val); }

aux_err aux_channel_set_size(aux_channel ch, const char *key, size_t val)
    { return (aux_err)nng_socket_set_size(ch.inner, key, val); }

aux_err aux_channel_set_duration(aux_channel ch, const char *key, aux_duration val)
    { return (aux_err)nng_socket_set_ms(ch.inner, key, val); }

aux_err aux_channel_get_bool(aux_channel ch, const char *key, bool *val)
    { return (aux_err)nng_socket_get_bool(ch.inner, key, val); }

aux_err aux_channel_get_int(aux_channel ch, const char *key, int *val)
    { return (aux_err)nng_socket_get_int(ch.inner, key, val); }

aux_err aux_channel_get_size(aux_channel ch, const char *key, size_t *val)
    { return (aux_err)nng_socket_get_size(ch.inner, key, val); }

aux_err aux_channel_get_duration(aux_channel ch, const char *key, aux_duration *val)
    { return (aux_err)nng_socket_get_ms(ch.inner, key, val); }

/* ================================================================
 * 通道元信息
 * ================================================================ */

aux_err aux_channel_proto_id(aux_channel ch, uint16_t *id)
    { return (aux_err)nng_socket_proto_id(ch.inner, id); }

aux_err aux_channel_peer_id(aux_channel ch, uint16_t *id)
    { return (aux_err)nng_socket_peer_id(ch.inner, id); }

aux_err aux_channel_proto_name(aux_channel ch, const char **name)
    { return (aux_err)nng_socket_proto_name(ch.inner, name); }

aux_err aux_channel_peer_name(aux_channel ch, const char **name)
    { return (aux_err)nng_socket_peer_name(ch.inner, name); }

aux_err aux_channel_is_raw(aux_channel ch, bool *raw)
    { return (aux_err)nng_socket_raw(ch.inner, raw); }

aux_err aux_channel_get_recv_poll_fd(aux_channel ch, int *fd)
    { return (aux_err)nng_socket_get_recv_poll_fd(ch.inner, fd); }

aux_err aux_channel_get_send_poll_fd(aux_channel ch, int *fd)
    { return (aux_err)nng_socket_get_send_poll_fd(ch.inner, fd); }
