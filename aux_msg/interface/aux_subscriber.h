/*
 * aux_subscriber.h - 订阅者专用 API（Subscriber 领域扩展）
 *
 * 封装 PUB/SUB 协议特有的订阅/取消订阅操作。
 *
 * Copyright (c) 2025 - Licensed under MIT
 */

#ifndef AUX_SUBSCRIBER_H
#define AUX_SUBSCRIBER_H

#include "aux_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NNG_HAVE_SUB0

// 通道级别的订阅
aux_err aux_subscriber_subscribe(aux_channel ch, const void *pattern, size_t len);
aux_err aux_subscriber_unsubscribe(aux_channel ch, const void *pattern, size_t len);

// 会话级别的订阅
aux_err aux_session_subscribe(aux_session sess, const void *pattern, size_t len);
aux_err aux_session_unsubscribe(aux_session sess, const void *pattern, size_t len);

// 订阅所有消息
static inline aux_err aux_subscriber_subscribe_all(aux_channel ch)
    { return aux_subscriber_subscribe(ch, "", 0); }

// 取消订阅所有
static inline aux_err aux_subscriber_unsubscribe_all(aux_channel ch)
    { return aux_subscriber_unsubscribe(ch, "", 0); }

#else  /* !NNG_HAVE_SUB0 */

// SUB0 未启用时提供桩函数，始终返回 AUX_ERR_NOT_SUPPORTED
static inline aux_err
aux_subscriber_subscribe(aux_channel ch, const void *pattern, size_t len)
    { (void)ch; (void)pattern; (void)len; return AUX_ERR_NOT_SUPPORTED; }

static inline aux_err
aux_subscriber_unsubscribe(aux_channel ch, const void *pattern, size_t len)
    { (void)ch; (void)pattern; (void)len; return AUX_ERR_NOT_SUPPORTED; }

static inline aux_err
aux_session_subscribe(aux_session sess, const void *pattern, size_t len)
    { (void)sess; (void)pattern; (void)len; return AUX_ERR_NOT_SUPPORTED; }

static inline aux_err
aux_session_unsubscribe(aux_session sess, const void *pattern, size_t len)
    { (void)sess; (void)pattern; (void)len; return AUX_ERR_NOT_SUPPORTED; }

static inline aux_err
aux_subscriber_subscribe_all(aux_channel ch)
    { (void)ch; return AUX_ERR_NOT_SUPPORTED; }

static inline aux_err
aux_subscriber_unsubscribe_all(aux_channel ch)
    { (void)ch; return AUX_ERR_NOT_SUPPORTED; }

#endif /* NNG_HAVE_SUB0 */

#ifdef __cplusplus
}
#endif

#endif /* AUX_SUBSCRIBER_H */
