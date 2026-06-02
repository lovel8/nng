/*
 * aux_channel.h - 通信通道（Channel）领域 API
 *
 * 装饰器模式封装 nng_socket 操作，提供 DDD 统一接口。
 * 所有导出接口名以 aux_ 开头。
 *
 * Copyright (c) 2025 - Licensed under MIT
 */

#ifndef AUX_CHANNEL_H
#define AUX_CHANNEL_H

#include "aux_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * 生命周期管理
 * ================================================================ */

// 打开一个通信通道，指定角色（协议类型）
aux_err aux_channel_open(aux_channel *ch, aux_channel_role role);

// 打开一个原始模式通道（raw mode，用于设备/代理）
aux_err aux_channel_open_raw(aux_channel *ch, aux_channel_role role);

// 关闭通信通道，释放所有资源
aux_err aux_channel_close(aux_channel ch);

// 获取通道内部 ID（用于调试），无效通道返回 -1
int aux_channel_id(aux_channel ch);

/* ================================================================
 * 通信能力
 * ================================================================ */

// 同步发送数据
aux_err aux_channel_send(aux_channel ch, const void *data, size_t size, int flags);

// 同步接收数据，actual_size 返回实际接收的字节数
aux_err aux_channel_recv(aux_channel ch, void *data, size_t *actual_size, int flags);

// 异步发送（通过 Future）
void aux_channel_send_async(aux_channel ch, aux_future *fut);

// 异步接收（通过 Future）
void aux_channel_recv_async(aux_channel ch, aux_future *fut);

/* ================================================================
 * 消息（Envelope）收发
 * ================================================================ */

// 同步发送消息信封
aux_err aux_channel_send_envelope(aux_channel ch, aux_envelope *env, int flags, aux_priority priority);

// 同步接收消息信封
aux_err aux_channel_recv_envelope(aux_channel ch, aux_envelope *env, int flags);

/* ================================================================
 * 连接管理
 * ================================================================ */

// 让通道在指定地址上接收连接（返回 Acceptor）
aux_err aux_channel_accept_at(
    aux_channel ch, const char *addr, aux_acceptor *acceptor, int flags);

// 让通道主动连接到指定地址（返回 Connector）
aux_err aux_channel_connect_to(
    aux_channel ch, const char *addr, aux_connector *connector, int flags);

// 通过 URI 接收连接
aux_err aux_channel_accept_at_uri(
    aux_channel ch, const aux_uri *uri, aux_acceptor *acceptor, int flags);

// 通过 URI 发起连接
aux_err aux_channel_connect_to_uri(
    aux_channel ch, const aux_uri *uri, aux_connector *connector, int flags);

/* ================================================================
 * 连接事件通知
 * ================================================================ */

aux_err aux_channel_on_connection_event(
    aux_channel ch, aux_conn_event event, aux_conn_callback cb, void *arg);

/* ================================================================
 * 选项读写
 * ================================================================ */

aux_err aux_channel_set_bool(aux_channel ch, const char *key, bool val);
aux_err aux_channel_set_int(aux_channel ch, const char *key, int val);
aux_err aux_channel_set_size(aux_channel ch, const char *key, size_t val);
aux_err aux_channel_set_duration(aux_channel ch, const char *key, aux_duration val);

aux_err aux_channel_get_bool(aux_channel ch, const char *key, bool *val);
aux_err aux_channel_get_int(aux_channel ch, const char *key, int *val);
aux_err aux_channel_get_size(aux_channel ch, const char *key, size_t *val);
aux_err aux_channel_get_duration(aux_channel ch, const char *key, aux_duration *val);

/* ================================================================
 * 通道元信息
 * ================================================================ */

// 获取本地协议 ID
aux_err aux_channel_proto_id(aux_channel ch, uint16_t *id);

// 获取对端协议 ID
aux_err aux_channel_peer_id(aux_channel ch, uint16_t *id);

// 获取本地协议名称
aux_err aux_channel_proto_name(aux_channel ch, const char **name);

// 获取对端协议名称
aux_err aux_channel_peer_name(aux_channel ch, const char **name);

// 是否为 raw 模式
aux_err aux_channel_is_raw(aux_channel ch, bool *raw);

/* ================================================================
 * 轮询文件描述符（集成外部事件循环）
 * ================================================================ */

aux_err aux_channel_get_recv_poll_fd(aux_channel ch, int *fd);
aux_err aux_channel_get_send_poll_fd(aux_channel ch, int *fd);

/* ================================================================
 * 便捷构造：一步打开并绑定/连接
 * ================================================================ */

// 创建发布者通道并绑定地址
static inline aux_err
aux_publisher_open(aux_channel *ch, const char *addr)
{
    aux_err rv;
    if ((rv = aux_channel_open(ch, AUX_ROLE_PUBLISHER)) != AUX_OK) {
        return rv;
    }
    return aux_channel_accept_at(*ch, addr, NULL, 0);
}

// 创建订阅者通道并连接地址
static inline aux_err
aux_subscriber_open(aux_channel *ch, const char *addr)
{
    aux_err rv;
    if ((rv = aux_channel_open(ch, AUX_ROLE_SUBSCRIBER)) != AUX_OK) {
        return rv;
    }
    return aux_channel_connect_to(*ch, addr, NULL, 0);
}

// 创建请求者通道并连接地址
static inline aux_err
aux_requester_open(aux_channel *ch, const char *addr)
{
    aux_err rv;
    if ((rv = aux_channel_open(ch, AUX_ROLE_REQUESTER)) != AUX_OK) {
        return rv;
    }
    return aux_channel_connect_to(*ch, addr, NULL, 0);
}

// 创建回复者通道并绑定地址
static inline aux_err
aux_replier_open(aux_channel *ch, const char *addr)
{
    aux_err rv;
    if ((rv = aux_channel_open(ch, AUX_ROLE_REPLIER)) != AUX_OK) {
        return rv;
    }
    return aux_channel_accept_at(*ch, addr, NULL, 0);
}

// 创建推送者通道并连接地址
static inline aux_err
aux_pusher_open(aux_channel *ch, const char *addr)
{
    aux_err rv;
    if ((rv = aux_channel_open(ch, AUX_ROLE_PUSHER)) != AUX_OK) {
        return rv;
    }
    return aux_channel_connect_to(*ch, addr, NULL, 0);
}

// 创建拉取者通道并绑定地址
static inline aux_err
aux_puller_open(aux_channel *ch, const char *addr)
{
    aux_err rv;
    if ((rv = aux_channel_open(ch, AUX_ROLE_PULLER)) != AUX_OK) {
        return rv;
    }
    return aux_channel_accept_at(*ch, addr, NULL, 0);
}

// 创建调查者通道并连接地址
static inline aux_err
aux_surveyor_open(aux_channel *ch, const char *addr)
{
    aux_err rv;
    if ((rv = aux_channel_open(ch, AUX_ROLE_SURVEYOR)) != AUX_OK) {
        return rv;
    }
    return aux_channel_connect_to(*ch, addr, NULL, 0);
}

// 创建响应者通道并绑定地址
static inline aux_err
aux_respondent_open(aux_channel *ch, const char *addr)
{
    aux_err rv;
    if ((rv = aux_channel_open(ch, AUX_ROLE_RESPONDENT)) != AUX_OK) {
        return rv;
    }
    return aux_channel_accept_at(*ch, addr, NULL, 0);
}

// 创建对等通道 v1 并监听
static inline aux_err
aux_peer_open(aux_channel *ch, const char *addr)
{
    aux_err rv;
    if ((rv = aux_channel_open(ch, AUX_ROLE_PEER_PAIR1)) != AUX_OK) {
        return rv;
    }
    return aux_channel_accept_at(*ch, addr, NULL, 0);
}

// 创建对等通道 v1 并连接
static inline aux_err
aux_peer_connect(aux_channel *ch, const char *addr)
{
    aux_err rv;
    if ((rv = aux_channel_open(ch, AUX_ROLE_PEER_PAIR1)) != AUX_OK) {
        return rv;
    }
    return aux_channel_connect_to(*ch, addr, NULL, 0);
}

#ifdef __cplusplus
}
#endif

#endif /* AUX_CHANNEL_H */
