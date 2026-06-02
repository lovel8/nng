/*
 * aux_envelope.c - 消息信封（Envelope）领域实现
 *
 * Copyright (c) 2025 - Licensed under MIT
 */

#include "aux_envelope.h"

/* ================================================================
 * 生命周期
 * ================================================================ */

aux_err
aux_envelope_alloc(aux_envelope *env, size_t size)
{
    if (!env) return AUX_ERR_INVALID_ARG;
    return (aux_err)nng_msg_alloc(&env->inner, size);
}

void
aux_envelope_free(aux_envelope *env)
{
    if (env && env->inner) {
        nng_msg_free(env->inner);
        env->inner = NULL;
    }
}

aux_err
aux_envelope_dup(aux_envelope *dst, const aux_envelope *src)
{
    if (!dst || !src) return AUX_ERR_INVALID_ARG;
    return (aux_err)nng_msg_dup(&dst->inner, src->inner);
}

void
aux_envelope_clear(aux_envelope *env)
{
    if (env && env->inner) nng_msg_clear(env->inner);
}

/* ================================================================
 * 容量与大小
 * ================================================================ */

aux_err aux_envelope_realloc(aux_envelope *env, size_t size)
    { return (aux_err)nng_msg_realloc(env->inner, size); }

aux_err aux_envelope_reserve(aux_envelope *env, size_t size)
    { return (aux_err)nng_msg_reserve(env->inner, size); }

size_t aux_envelope_capacity(const aux_envelope *env)
    { return nng_msg_capacity(env->inner); }

size_t aux_envelope_body_len(const aux_envelope *env)
    { return nng_msg_len(env->inner); }

size_t aux_envelope_header_len(const aux_envelope *env)
    { return nng_msg_header_len(env->inner); }

/* ================================================================
 * 头部操作
 * ================================================================ */

void *aux_envelope_header(aux_envelope *env)
    { return nng_msg_header(env->inner); }

void aux_envelope_header_clear(aux_envelope *env)
    { nng_msg_header_clear(env->inner); }

aux_err aux_envelope_header_append(aux_envelope *env, const void *data, size_t size)
    { return (aux_err)nng_msg_header_append(env->inner, data, size); }

aux_err aux_envelope_header_insert(aux_envelope *env, const void *data, size_t size)
    { return (aux_err)nng_msg_header_insert(env->inner, data, size); }

aux_err aux_envelope_header_trim(aux_envelope *env, size_t size)
    { return (aux_err)nng_msg_header_trim(env->inner, size); }

aux_err aux_envelope_header_chop(aux_envelope *env, size_t size)
    { return (aux_err)nng_msg_header_chop(env->inner, size); }

aux_err aux_envelope_header_append_u16(aux_envelope *env, uint16_t val)
    { return (aux_err)nng_msg_header_append_u16(env->inner, val); }

aux_err aux_envelope_header_append_u32(aux_envelope *env, uint32_t val)
    { return (aux_err)nng_msg_header_append_u32(env->inner, val); }

aux_err aux_envelope_header_append_u64(aux_envelope *env, uint64_t val)
    { return (aux_err)nng_msg_header_append_u64(env->inner, val); }

aux_err aux_envelope_header_trim_u16(aux_envelope *env, uint16_t *val)
    { return (aux_err)nng_msg_header_trim_u16(env->inner, val); }

aux_err aux_envelope_header_trim_u32(aux_envelope *env, uint32_t *val)
    { return (aux_err)nng_msg_header_trim_u32(env->inner, val); }

aux_err aux_envelope_header_trim_u64(aux_envelope *env, uint64_t *val)
    { return (aux_err)nng_msg_header_trim_u64(env->inner, val); }

aux_err aux_envelope_header_chop_u16(aux_envelope *env, uint16_t *val)
    { return (aux_err)nng_msg_header_chop_u16(env->inner, val); }

aux_err aux_envelope_header_chop_u32(aux_envelope *env, uint32_t *val)
    { return (aux_err)nng_msg_header_chop_u32(env->inner, val); }

aux_err aux_envelope_header_chop_u64(aux_envelope *env, uint64_t *val)
    { return (aux_err)nng_msg_header_chop_u64(env->inner, val); }

/* ================================================================
 * 消息体操作
 * ================================================================ */

void *aux_envelope_body(aux_envelope *env)
    { return nng_msg_body(env->inner); }

aux_err aux_envelope_body_append(aux_envelope *env, const void *data, size_t size)
    { return (aux_err)nng_msg_append(env->inner, data, size); }

aux_err aux_envelope_body_insert(aux_envelope *env, const void *data, size_t size)
    { return (aux_err)nng_msg_insert(env->inner, data, size); }

aux_err aux_envelope_body_trim(aux_envelope *env, size_t size)
    { return (aux_err)nng_msg_trim(env->inner, size); }

aux_err aux_envelope_body_chop(aux_envelope *env, size_t size)
    { return (aux_err)nng_msg_chop(env->inner, size); }

aux_err aux_envelope_body_append_u16(aux_envelope *env, uint16_t val)
    { return (aux_err)nng_msg_append_u16(env->inner, val); }

aux_err aux_envelope_body_append_u32(aux_envelope *env, uint32_t val)
    { return (aux_err)nng_msg_append_u32(env->inner, val); }

aux_err aux_envelope_body_append_u64(aux_envelope *env, uint64_t val)
    { return (aux_err)nng_msg_append_u64(env->inner, val); }

aux_err aux_envelope_body_trim_u16(aux_envelope *env, uint16_t *val)
    { return (aux_err)nng_msg_trim_u16(env->inner, val); }

aux_err aux_envelope_body_trim_u32(aux_envelope *env, uint32_t *val)
    { return (aux_err)nng_msg_trim_u32(env->inner, val); }

aux_err aux_envelope_body_trim_u64(aux_envelope *env, uint64_t *val)
    { return (aux_err)nng_msg_trim_u64(env->inner, val); }

aux_err aux_envelope_body_chop_u16(aux_envelope *env, uint16_t *val)
    { return (aux_err)nng_msg_chop_u16(env->inner, val); }

aux_err aux_envelope_body_chop_u32(aux_envelope *env, uint32_t *val)
    { return (aux_err)nng_msg_chop_u32(env->inner, val); }

aux_err aux_envelope_body_chop_u64(aux_envelope *env, uint64_t *val)
    { return (aux_err)nng_msg_chop_u64(env->inner, val); }

/* ================================================================
 * 关联信息
 * ================================================================ */

void
aux_envelope_set_connection(aux_envelope *env, aux_connection conn)
{
    nng_msg_set_pipe(env->inner, conn.inner);
}

aux_connection
aux_envelope_get_connection(const aux_envelope *env)
{
    aux_connection conn;
    conn.inner = nng_msg_get_pipe(env->inner);
    return conn;
}
