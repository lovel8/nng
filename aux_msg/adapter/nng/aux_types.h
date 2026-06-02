/*
 * aux_types.h - 适配层类型映射（与底层库绑定）
 *
 * 将底层库（当前为 NNG）的具体类型映射为 aux_* 领域类型。
 * 替换底层库时只需修改此文件中的映射关系。
 *
 * 包含内容：
 *   - 结构体定义（aux_channel 等，内嵌 nng_* 类型）
 *   - 错误码映射（aux_err → NNG_*）
 *   - 宏映射（AUX_FOREVER → NNG_DURATION_INFINITE 等）
 *   - 选项宏映射（AUX_OPT_* → NNG_OPT_*）
 *   - 类型别名（aux_net_addr → nng_sockaddr 等）
 *   - 初始化参数结构体
 *
 * Copyright (c) 2025 - Licensed under MIT
 */

#ifndef AUX_NNG_TYPES_H
#define AUX_NNG_TYPES_H

#include <nng/nng.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ================================================================
 * 基础领域类型（装饰 NNG 原生类型）
 * ================================================================ */

// 通信通道 —— 装饰 nng_socket
typedef struct aux_channel {
    nng_socket inner;
} aux_channel;

// 连接器（主动发起连接） —— 装饰 nng_dialer
typedef struct aux_connector {
    nng_dialer inner;
} aux_connector;

// 接收器（被动接受连接） —— 装饰 nng_listener
typedef struct aux_acceptor {
    nng_listener inner;
} aux_acceptor;

// 连接实例 —— 装饰 nng_pipe
typedef struct aux_connection {
    nng_pipe inner;
} aux_connection;

// 会话（多路复用上下文） —— 装饰 nng_ctx
typedef struct aux_session {
    nng_ctx inner;
} aux_session;

// 消息信封 —— 装饰 nng_msg*
typedef struct aux_envelope {
    nng_msg *inner;
} aux_envelope;

// 异步 Future —— 装饰 nng_aio*
typedef struct aux_future {
    nng_aio *inner;
} aux_future;

// 字节流 —— 装饰 nng_stream*
typedef struct aux_byte_stream {
    nng_stream *inner;
} aux_byte_stream;

// 流连接器 —— 装饰 nng_stream_dialer*
typedef struct aux_stream_connector {
    nng_stream_dialer *inner;
} aux_stream_connector;

// 流接收器 —— 装饰 nng_stream_listener*
typedef struct aux_stream_acceptor {
    nng_stream_listener *inner;
} aux_stream_acceptor;

// 网络地址
typedef nng_sockaddr      aux_net_addr;
typedef nng_sockaddr_in   aux_ipv4_addr;
typedef nng_sockaddr_in6  aux_ipv6_addr;
typedef nng_sockaddr_ipc  aux_ipc_addr;

// 网络 URI
typedef nng_url aux_uri;

// I/O 向量
typedef nng_iov aux_io_vec;

/* ================================================================
 * 时间相关宏
 * ================================================================ */

#define AUX_FOREVER   NNG_DURATION_INFINITE
#define AUX_DEFAULT   NNG_DURATION_DEFAULT
#define AUX_IMMEDIATE NNG_DURATION_ZERO

/* ================================================================
 * 错误码（DDD 语义化 → NNG 原生值映射）
 * ================================================================ */

typedef enum {
    AUX_OK                = NNG_OK,
    AUX_ERR_INTERRUPTED   = NNG_EINTR,
    AUX_ERR_NO_MEMORY     = NNG_ENOMEM,
    AUX_ERR_INVALID_ARG   = NNG_EINVAL,
    AUX_ERR_BUSY          = NNG_EBUSY,
    AUX_ERR_TIMEOUT       = NNG_ETIMEDOUT,
    AUX_ERR_REFUSED       = NNG_ECONNREFUSED,
    AUX_ERR_CLOSED        = NNG_ECLOSED,
    AUX_ERR_AGAIN         = NNG_EAGAIN,
    AUX_ERR_NOT_SUPPORTED = NNG_ENOTSUP,
    AUX_ERR_ADDR_IN_USE   = NNG_EADDRINUSE,
    AUX_ERR_BAD_STATE     = NNG_ESTATE,
    AUX_ERR_NOT_FOUND     = NNG_ENOENT,
    AUX_ERR_PROTOCOL      = NNG_EPROTO,
    AUX_ERR_UNREACHABLE   = NNG_EUNREACHABLE,
    AUX_ERR_BAD_ADDR      = NNG_EADDRINVAL,
    AUX_ERR_PERMISSION    = NNG_EPERM,
    AUX_ERR_MSG_TOO_BIG   = NNG_EMSGSIZE,
    AUX_ERR_CONN_ABORTED  = NNG_ECONNABORTED,
    AUX_ERR_CONN_RESET    = NNG_ECONNRESET,
    AUX_ERR_CANCELED      = NNG_ECANCELED,
    AUX_ERR_NO_FILES      = NNG_ENOFILES,
    AUX_ERR_NO_SPACE      = NNG_ENOSPC,
    AUX_ERR_EXISTS        = NNG_EEXIST,
    AUX_ERR_READ_ONLY     = NNG_EREADONLY,
    AUX_ERR_WRITE_ONLY    = NNG_EWRITEONLY,
    AUX_ERR_CRYPTO        = NNG_ECRYPTO,
    AUX_ERR_PEER_AUTH     = NNG_EPEERAUTH,
    AUX_ERR_BAD_TYPE      = NNG_EBADTYPE,
    AUX_ERR_CONN_SHUTDOWN = NNG_ECONNSHUT,
    AUX_ERR_STOPPED       = NNG_ESTOPPED,
    AUX_ERR_INTERNAL      = NNG_EINTERNAL,
    AUX_ERR_SYSTEM        = NNG_ESYSERR,
    AUX_ERR_TRANSPORT     = NNG_ETRANERR,
} aux_err;

/* ================================================================
 * 选项键名映射
 * ================================================================ */

#define AUX_OPT_RECV_BUFFER     NNG_OPT_RECVBUF
#define AUX_OPT_SEND_BUFFER     NNG_OPT_SENDBUF
#define AUX_OPT_RECV_TIMEOUT    NNG_OPT_RECVTIMEO
#define AUX_OPT_SEND_TIMEOUT    NNG_OPT_SENDTIMEO
#define AUX_OPT_LOCAL_ADDR      NNG_OPT_LOCADDR
#define AUX_OPT_MAX_TTL         NNG_OPT_MAXTTL
#define AUX_OPT_RECV_MAX_SIZE   NNG_OPT_RECVMAXSZ
#define AUX_OPT_RECONN_MIN_TIME NNG_OPT_RECONNMINT
#define AUX_OPT_RECONN_MAX_TIME NNG_OPT_RECONNMAXT
#define AUX_OPT_TCP_NODELAY     NNG_OPT_TCP_NODELAY
#define AUX_OPT_TCP_KEEPALIVE   NNG_OPT_TCP_KEEPALIVE
#define AUX_OPT_BOUND_PORT      NNG_OPT_BOUND_PORT

/* ================================================================
 * 通道地址最大长度
 * ================================================================ */

#define AUX_MAX_ADDR_LEN    NNG_MAXADDRLEN
#define AUX_MAX_ADDR_STRLEN NNG_MAXADDRSTRLEN

/* ================================================================
 * 标记
 * ================================================================ */

#define AUX_FLAG_NONBLOCK NNG_FLAG_NONBLOCK

/* ================================================================
 * 初始化参数
 * ================================================================ */

typedef struct {
    int16_t num_task_threads;
    int16_t max_task_threads;
    int16_t num_expire_threads;
    int16_t max_expire_threads;
    int16_t num_poller_threads;
    int16_t max_poller_threads;
    int16_t num_resolver_threads;
} aux_init_params;

#define AUX_INIT_PARAMS_DEFAULT \
    { 0, 0, 0, 0, 0, 0, 0 }

#endif /* AUX_NNG_TYPES_H */
