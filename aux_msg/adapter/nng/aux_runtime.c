/*
 * aux_runtime.c - 运行时领域实现
 *
 * Copyright (c) 2025 - Licensed under MIT
 */

#include "aux_runtime.h"

#include <string.h>

/* ================================================================
 * 库生命周期
 * ================================================================ */

aux_err aux_runtime_init(const aux_init_params *params)
    { return (aux_err)nng_init((const nng_init_params *)params); }

void aux_runtime_fini(void) { nng_fini(); }

const char *aux_runtime_version(void) { return nng_version(); }

/* ================================================================
 * 错误信息
 * ================================================================ */

const char *aux_strerror(aux_err err) { return nng_strerror((int)err); }

/* ================================================================
 * 时间与休眠
 * ================================================================ */

aux_timestamp aux_clock(void) { return nng_clock(); }

void aux_sleep(aux_duration ms) { nng_msleep(ms); }

/* ================================================================
 * 随机数
 * ================================================================ */

uint32_t aux_random(void) { return nng_random(); }

/* ================================================================
 * URI 操作
 * ================================================================ */

aux_err aux_uri_parse(aux_uri **uri, const char *str)
    { return (aux_err)nng_url_parse(uri, str); }

void aux_uri_free(aux_uri *uri) { nng_url_free(uri); }

aux_err aux_uri_clone(aux_uri **dst, const aux_uri *src)
    { return (aux_err)nng_url_clone(dst, src); }

int aux_uri_format(char *buf, size_t size, const aux_uri *uri)
    { return nng_url_sprintf(buf, size, uri); }

const char *aux_uri_scheme(const aux_uri *uri)    { return nng_url_scheme(uri); }
const char *aux_uri_hostname(const aux_uri *uri)  { return nng_url_hostname(uri); }
const char *aux_uri_path(const aux_uri *uri)      { return nng_url_path(uri); }
const char *aux_uri_query(const aux_uri *uri)     { return nng_url_query(uri); }
const char *aux_uri_fragment(const aux_uri *uri)  { return nng_url_fragment(uri); }
const char *aux_uri_userinfo(const aux_uri *uri)  { return nng_url_userinfo(uri); }
uint32_t    aux_uri_port(const aux_uri *uri)      { return nng_url_port(uri); }

void aux_uri_resolve_port(aux_uri *uri, uint32_t port)
    { nng_url_resolve_port(uri, port); }

/* ================================================================
 * 网络地址操作
 * ================================================================ */

const char *aux_net_addr_to_string(const aux_net_addr *addr, char *buf, size_t size)
    { return nng_str_sockaddr(addr, buf, size); }

uint32_t aux_net_addr_port(const aux_net_addr *addr)
    { return nng_sockaddr_port(addr); }

bool aux_net_addr_equal(const aux_net_addr *a, const aux_net_addr *b)
    { return nng_sockaddr_equal(a, b); }

uint64_t aux_net_addr_hash(const aux_net_addr *addr)
    { return nng_sockaddr_hash(addr); }

/* ================================================================
 * 内存管理
 * ================================================================ */

void *aux_alloc(size_t size) { return nng_alloc(size); }
void  aux_free(void *ptr, size_t size) { nng_free(ptr, size); }
char *aux_strdup(const char *str) { return nng_strdup(str); }
void  aux_strfree(char *str) { nng_strfree(str); }

/* ================================================================
 * 设备
 * ================================================================ */

aux_err aux_device_forward(aux_channel ch1, aux_channel ch2)
    { return (aux_err)nng_device(ch1.inner, ch2.inner); }

void aux_device_forward_async(aux_future *fut, aux_channel ch1, aux_channel ch2)
    { nng_device_aio(fut->inner, ch1.inner, ch2.inner); }

/* ================================================================
 * Socket Pair
 * ================================================================ */

aux_err aux_socket_pair(int fds[2])
    { return (aux_err)nng_socket_pair(fds); }
