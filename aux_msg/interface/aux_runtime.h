/*
 * aux_runtime.h - 运行时（Runtime）领域 API
 *
 * 封装 NNG 库初始化、版本、工具函数、URI、网络地址等。
 *
 * Copyright (c) 2025 - Licensed under MIT
 */

#ifndef AUX_RUNTIME_H
#define AUX_RUNTIME_H

#include "aux_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * 库生命周期
 * ================================================================ */

// 初始化 NNG 运行时
aux_err aux_runtime_init(const aux_init_params *params);

// 终止 NNG 运行时
void aux_runtime_fini(void);

// 获取库版本字符串
const char *aux_runtime_version(void);

/* ================================================================
 * 错误信息
 * ================================================================ */

// 获取错误码的描述字符串
const char *aux_strerror(aux_err err);

/* ================================================================
 * 时间与休眠
 * ================================================================ */

// 获取当前时间戳（毫秒）
aux_timestamp aux_clock(void);

// 休眠指定毫秒
void aux_sleep(aux_duration ms);

/* ================================================================
 * 随机数
 * ================================================================ */

// 生成加密级随机数
uint32_t aux_random(void);

/* ================================================================
 * URI 操作
 * ================================================================ */

// 解析 URI 字符串
aux_err aux_uri_parse(aux_uri **uri, const char *str);

// 释放 URI
void aux_uri_free(aux_uri *uri);

// 复制 URI
aux_err aux_uri_clone(aux_uri **dst, const aux_uri *src);

// 将 URI 格式化为字符串
int aux_uri_format(char *buf, size_t size, const aux_uri *uri);

// 获取 URI 各组成部分
const char *aux_uri_scheme(const aux_uri *uri);
const char *aux_uri_hostname(const aux_uri *uri);
const char *aux_uri_path(const aux_uri *uri);
const char *aux_uri_query(const aux_uri *uri);
const char *aux_uri_fragment(const aux_uri *uri);
const char *aux_uri_userinfo(const aux_uri *uri);
uint32_t    aux_uri_port(const aux_uri *uri);

// 更新端口（仅当端口为 0 时生效）
void aux_uri_resolve_port(aux_uri *uri, uint32_t port);

/* ================================================================
 * 网络地址操作
 * ================================================================ */

// 将地址转换为可打印字符串
const char *aux_net_addr_to_string(
    const aux_net_addr *addr, char *buf, size_t size);

// 获取端口号
uint32_t aux_net_addr_port(const aux_net_addr *addr);

// 比较两个地址是否相等
bool aux_net_addr_equal(const aux_net_addr *a, const aux_net_addr *b);

// 计算地址哈希值
uint64_t aux_net_addr_hash(const aux_net_addr *addr);

/* ================================================================
 * 内存管理
 * ================================================================ */

void *aux_alloc(size_t size);
void  aux_free(void *ptr, size_t size);
char *aux_strdup(const char *str);
void  aux_strfree(char *str);

/* ================================================================
 * 设备（Device）- 双向转发两个通道
 * ================================================================ */

// 同步设备：阻塞直到任一端关闭
aux_err aux_device_forward(aux_channel ch1, aux_channel ch2);

// 异步设备：后台运行
void aux_device_forward_async(aux_future *fut, aux_channel ch1, aux_channel ch2);

/* ================================================================
 * 便捷：Socket Pair
 * ================================================================ */

// 创建一对已连接的文件描述符
aux_err aux_socket_pair(int fds[2]);

#ifdef __cplusplus
}
#endif

#endif /* AUX_RUNTIME_H */
