/*
 * aux_future.h - 异步 Future 领域 API
 *
 * 装饰器模式封装 nng_aio 操作。
 * DDD 概念：Future（异步操作句柄），代表一个尚未完成的异步操作。
 *
 * Copyright (c) 2025 - Licensed under MIT
 */

#ifndef AUX_FUTURE_H
#define AUX_FUTURE_H

#include "aux_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * 生命周期
 * ================================================================ */

// 分配 Future，可选回调（callback=NULL 则同步等待）
aux_err aux_future_alloc(aux_future *fut, aux_future_callback cb, void *arg);

// 释放 Future（不能在使用中释放）
void aux_future_free(aux_future *fut);

// 异步安全释放（可从回调中调用）
void aux_future_reap(aux_future *fut);

// 停止 Future（永久性，不能再提交操作）
void aux_future_stop(aux_future *fut);

/* ================================================================
 * 状态查询
 * ================================================================ */

// 获取操作结果（成功返回 AUX_OK）
aux_err aux_future_result(aux_future *fut);

// 获取传输字节数（send/recv 操作）
size_t aux_future_count(aux_future *fut);

// 是否正在处理中
bool aux_future_is_busy(aux_future *fut);

/* ================================================================
 * 控制操作
 * ================================================================ */

// 取消正在进行的异步操作
void aux_future_cancel(aux_future *fut);

// 取消并指定错误码
void aux_future_abort(aux_future *fut, aux_err reason);

// 同步等待操作完成
void aux_future_wait(aux_future *fut);

/* ================================================================
 * 超时控制
 * ================================================================ */

// 设置相对超时时间
void aux_future_set_timeout(aux_future *fut, aux_duration timeout);

// 设置绝对过期时间
void aux_future_set_expire(aux_future *fut, aux_timestamp expire);

/* ================================================================
 * 数据传递
 * ================================================================ */

// 设置关联的消息信封（用于发送操作）
void aux_future_set_envelope(aux_future *fut, aux_envelope *env);

// 获取关联的消息信封（接收完成后）
aux_envelope aux_future_get_envelope(aux_future *fut);

// 设置 I/O 向量（用于 stream send/recv）
aux_err aux_future_set_io_vec(aux_future *fut, unsigned n, const aux_io_vec *iov);

// 设置输入参数
aux_err aux_future_set_input(aux_future *fut, unsigned index, void *val);

// 获取输入参数
void *aux_future_get_input(aux_future *fut, unsigned index);

// 获取输出参数
void *aux_future_get_output(aux_future *fut, unsigned index);

/* ================================================================
 * 提供者接口（用于自定义异步操作）
 * ================================================================ */

typedef void (*aux_future_cancel_fn)(aux_future *fut, void *arg, aux_err reason);

// 重置 Future（提供者在操作前调用）
void aux_future_reset(aux_future *fut);

// 开始异步操作（提供者调用）
bool aux_future_start(aux_future *fut, aux_future_cancel_fn cancel_fn, void *arg);

// 完成异步操作（提供者调用，必须恰好调用一次）
void aux_future_finish(aux_future *fut, aux_err result);

/* ================================================================
 * 便捷：延迟操作
 * ================================================================ */

// 在指定延迟后触发回调（结果总是 0）
void aux_future_sleep(aux_duration ms, aux_future *fut);

#ifdef __cplusplus
}
#endif

#endif /* AUX_FUTURE_H */
