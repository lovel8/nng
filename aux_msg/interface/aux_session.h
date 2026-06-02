/*
 * aux_session.h - 会话（Session）领域 API
 *
 * 装饰器模式封装 nng_ctx 操作。
 * DDD 概念：Session（会话）= 基于通道的多路复用上下文，
 * 每个 Session 拥有独立的状态机，实现并发协议处理。
 *
 * Copyright (c) 2025 - Licensed under MIT
 */

#ifndef AUX_SESSION_H
#define AUX_SESSION_H

#include "aux_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * 生命周期
 * ================================================================ */

// 在通道上创建一个会话（需要协议支持，如 REQ/REP、SURVEYOR）
aux_err aux_session_open(aux_session *sess, aux_channel ch);

// 关闭会话
aux_err aux_session_close(aux_session sess);

// 获取会话 ID
int aux_session_id(aux_session sess);

/* ================================================================
 * 通信能力
 * ================================================================ */

// 异步发送（通过 Future）
void aux_session_send_async(aux_session sess, aux_future *fut);

// 异步接收（通过 Future）
void aux_session_recv_async(aux_session sess, aux_future *fut);

// 同步发送消息信封
aux_err aux_session_send_envelope(aux_session sess, aux_envelope *env, int flags, aux_priority priority);

// 同步接收消息信封
aux_err aux_session_recv_envelope(aux_session sess, aux_envelope *env, int flags);

/* ================================================================
 * 选项
 * ================================================================ */

aux_err aux_session_set_bool(aux_session sess, const char *key, bool val);
aux_err aux_session_set_int(aux_session sess, const char *key, int val);
aux_err aux_session_set_size(aux_session sess, const char *key, size_t val);
aux_err aux_session_set_duration(aux_session sess, const char *key, aux_duration val);

aux_err aux_session_get_bool(aux_session sess, const char *key, bool *val);
aux_err aux_session_get_int(aux_session sess, const char *key, int *val);
aux_err aux_session_get_size(aux_session sess, const char *key, size_t *val);
aux_err aux_session_get_duration(aux_session sess, const char *key, aux_duration *val);

#ifdef __cplusplus
}
#endif

#endif /* AUX_SESSION_H */
