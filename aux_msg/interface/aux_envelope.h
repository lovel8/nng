/*
 * aux_envelope.h - 消息信封（Envelope）领域 API
 *
 * 装饰器模式封装 nng_msg 操作。
 * DDD 概念：Message / Envelope（消息信封），包含 header + body。
 *
 * Copyright (c) 2025 - Licensed under MIT
 */

#ifndef AUX_ENVELOPE_H
#define AUX_ENVELOPE_H

#include "aux_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * 生命周期
 * ================================================================ */

// 分配指定大小的消息信封
aux_err aux_envelope_alloc(aux_envelope *env, size_t size);

// 释放消息信封
void aux_envelope_free(aux_envelope *env);

// 复制消息信封
aux_err aux_envelope_dup(aux_envelope *dst, const aux_envelope *src);

// 清空信封内容
void aux_envelope_clear(aux_envelope *env);

/* ================================================================
 * 容量与大小
 * ================================================================ */

// 重新分配底层缓冲区
aux_err aux_envelope_realloc(aux_envelope *env, size_t size);

// 预留头部空间（用于后续 header 操作）
aux_err aux_envelope_reserve(aux_envelope *env, size_t size);

// 获取缓冲区总容量
size_t aux_envelope_capacity(const aux_envelope *env);

// 获取消息体长度
size_t aux_envelope_body_len(const aux_envelope *env);

// 获取消息头长度
size_t aux_envelope_header_len(const aux_envelope *env);

/* ================================================================
 * 头部（Header）访问与操作
 * ================================================================ */

// 获取头部数据指针
void *aux_envelope_header(aux_envelope *env);

// 清空头部
void aux_envelope_header_clear(aux_envelope *env);

// 向头部追加数据
aux_err aux_envelope_header_append(aux_envelope *env, const void *data, size_t size);

// 向头部插入数据
aux_err aux_envelope_header_insert(aux_envelope *env, const void *data, size_t size);

// 从头部前端裁剪 size 字节
aux_err aux_envelope_header_trim(aux_envelope *env, size_t size);

// 从头部末尾截断 size 字节
aux_err aux_envelope_header_chop(aux_envelope *env, size_t size);

// 便捷：向头部追加 uint16/32/64
aux_err aux_envelope_header_append_u16(aux_envelope *env, uint16_t val);
aux_err aux_envelope_header_append_u32(aux_envelope *env, uint32_t val);
aux_err aux_envelope_header_append_u64(aux_envelope *env, uint64_t val);

// 便捷：从头部前端读取 uint16/32/64 并移除
aux_err aux_envelope_header_trim_u16(aux_envelope *env, uint16_t *val);
aux_err aux_envelope_header_trim_u32(aux_envelope *env, uint32_t *val);
aux_err aux_envelope_header_trim_u64(aux_envelope *env, uint64_t *val);

// 便捷：从头部末尾读取 uint16/32/64 并移除
aux_err aux_envelope_header_chop_u16(aux_envelope *env, uint16_t *val);
aux_err aux_envelope_header_chop_u32(aux_envelope *env, uint32_t *val);
aux_err aux_envelope_header_chop_u64(aux_envelope *env, uint64_t *val);

/* ================================================================
 * 消息体（Body）访问与操作
 * ================================================================ */

// 获取消息体数据指针
void *aux_envelope_body(aux_envelope *env);

// 向消息体末尾追加数据
aux_err aux_envelope_body_append(aux_envelope *env, const void *data, size_t size);

// 向消息体头部插入数据
aux_err aux_envelope_body_insert(aux_envelope *env, const void *data, size_t size);

// 从消息体前端裁剪 size 字节
aux_err aux_envelope_body_trim(aux_envelope *env, size_t size);

// 从消息体末尾截断 size 字节
aux_err aux_envelope_body_chop(aux_envelope *env, size_t size);

// 便捷：向消息体追加 uint16/32/64
aux_err aux_envelope_body_append_u16(aux_envelope *env, uint16_t val);
aux_err aux_envelope_body_append_u32(aux_envelope *env, uint32_t val);
aux_err aux_envelope_body_append_u64(aux_envelope *env, uint64_t val);

// 便捷：从消息体前端读取 uint16/32/64 并移除
aux_err aux_envelope_body_trim_u16(aux_envelope *env, uint16_t *val);
aux_err aux_envelope_body_trim_u32(aux_envelope *env, uint32_t *val);
aux_err aux_envelope_body_trim_u64(aux_envelope *env, uint64_t *val);

// 便捷：从消息体末尾读取 uint16/32/64 并移除
aux_err aux_envelope_body_chop_u16(aux_envelope *env, uint16_t *val);
aux_err aux_envelope_body_chop_u32(aux_envelope *env, uint32_t *val);
aux_err aux_envelope_body_chop_u64(aux_envelope *env, uint64_t *val);

/* ================================================================
 * 关联信息
 * ================================================================ */

// 设置消息所属的连接
void aux_envelope_set_connection(aux_envelope *env, aux_connection conn);

// 获取消息来源的连接
aux_connection aux_envelope_get_connection(const aux_envelope *env);

#ifdef __cplusplus
}
#endif

#endif /* AUX_ENVELOPE_H */
