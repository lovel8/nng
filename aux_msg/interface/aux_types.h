/*
 * aux_types.h - 统一接口层：纯领域类型定义（不依赖底层库）
 *
 * 此文件定义 DDD 领域驱动设计中的纯业务概念，与具体底层通信库解耦。
 * 替换底层库时此文件无需修改。
 *
 * 底层库相关的类型映射（结构体内嵌、宏映射等）见 adapter/nng/aux_types.h
 *
 * Copyright (c) 2025 - Licensed under MIT
 */

#ifndef AUX_TYPES_H
#define AUX_TYPES_H

#include <stdint.h>

/*
 * 符号可见性控制：
 * - macOS: 通过 -exported_symbols_list 只导出 aux_*
 * - Linux: 通过 --version-script 只导出 aux_*
 * - 所有符号默认可见，链接器负责裁剪
 */
#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * 基础类型（与底层库无关的纯领域概念）
 * ================================================================ */

typedef int32_t  aux_duration;   // 相对时间（毫秒）
typedef uint64_t aux_timestamp;  // 绝对时间（毫秒）

/* ================================================================
 * 通道角色（通信模式 / 协议类型）
 * ================================================================ */

typedef enum {
    AUX_ROLE_PUBLISHER,      // 发布者
    AUX_ROLE_SUBSCRIBER,     // 订阅者
    AUX_ROLE_REQUESTER,      // 请求者
    AUX_ROLE_REPLIER,        // 回复者
    AUX_ROLE_PUSHER,         // 推送者
    AUX_ROLE_PULLER,         // 拉取者
    AUX_ROLE_SURVEYOR,       // 调查者
    AUX_ROLE_RESPONDENT,     // 响应者
    AUX_ROLE_PEER_PAIR0,     // 对等通信 v0
    AUX_ROLE_PEER_PAIR1,     // 对等通信 v1
    AUX_ROLE_BUS,            // 总线参与者
} aux_channel_role;

/* ================================================================
 * 传输方式
 * ================================================================ */

typedef enum {
    AUX_TRANSPORT_TCP,       // tcp://
    AUX_TRANSPORT_TLS,       // tls+tcp://
    AUX_TRANSPORT_WS,        // ws://
    AUX_TRANSPORT_WSS,       // wss://
    AUX_TRANSPORT_IPC,       // ipc://
    AUX_TRANSPORT_INPROC,    // inproc://
    AUX_TRANSPORT_UDP,       // udp://
    AUX_TRANSPORT_DTLS,      // dtls+udp://
} aux_transport;

/* ================================================================
 * 前向声明 —— 结构体类型（具体定义在 adapter/nng/aux_types.h）
 * 必须在回调之前声明，因为回调使用 aux_connection 作为参数类型
 * ================================================================ */

typedef struct aux_channel     aux_channel;
typedef struct aux_connector   aux_connector;
typedef struct aux_acceptor    aux_acceptor;
typedef struct aux_connection  aux_connection;
typedef struct aux_session     aux_session;
typedef struct aux_envelope    aux_envelope;
typedef struct aux_future      aux_future;
typedef struct aux_byte_stream aux_byte_stream;
typedef struct aux_stream_connector aux_stream_connector;
typedef struct aux_stream_acceptor  aux_stream_acceptor;

/*
 * aux_init_params 在 adapter/nng/aux_types.h 中定义为匿名 struct 的 typedef，
 * 因此不在此处做前向声明。使用它的代码需先 include aux_msg.h。
 */

/*
 * 注意：以下类型在 adapter/nng/aux_types.h 中通过 typedef 直接映射到 NNG 类型，
 * 不是 struct，因此此处不做前向声明。使用它们的代码需先 include aux_msg.h。
 *   - aux_net_addr   → nng_sockaddr
 *   - aux_ipv4_addr  → nng_sockaddr_in
 *   - aux_ipv6_addr  → nng_sockaddr_in6
 *   - aux_ipc_addr   → nng_sockaddr_ipc
 *   - aux_uri        → nng_url
 *   - aux_io_vec     → nng_iov
 */

/* ================================================================
 * 连接事件回调
 * ================================================================ */

typedef enum {
    AUX_CONN_EVENT_ADD_PRE,   // 连接即将建立
    AUX_CONN_EVENT_ADD_POST,  // 连接已建立
    AUX_CONN_EVENT_REM_POST,  // 连接已移除
} aux_conn_event;

typedef void (*aux_conn_callback)(
    aux_connection conn, aux_conn_event event, void *arg);

/* ================================================================
 * Future 完成回调
 * ================================================================ */

typedef void (*aux_future_callback)(void *arg);

/*
 * 引入适配层类型映射，提供完整的结构体定义、错误码值、宏映射等。
 * 替换底层库时只需修改此 include 指向的适配文件。
 */
#include "adapter/nng/aux_types.h"

#ifdef __cplusplus
}
#endif

#endif /* AUX_TYPES_H */
