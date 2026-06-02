/*
 * aux_connector.h - 连接器与接收器领域 API
 *
 * 装饰器模式封装 nng_dialer / nng_listener 操作。
 * DDD 概念：
 *   Connector（连接器）= 主动发起连接的端点
 *   Acceptor（接收器）= 被动接受连接的端点
 *
 * Copyright (c) 2025 - Licensed under MIT
 */

#ifndef AUX_CONNECTOR_H
#define AUX_CONNECTOR_H

#include "aux_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * Connector（连接器）- 主动发起连接
 * ================================================================ */

// 创建连接器（不立即启动）
aux_err aux_connector_create(
    aux_connector *conn, aux_channel ch, const char *addr);

// 创建连接器（通过 URI）
aux_err aux_connector_create_by_uri(
    aux_connector *conn, aux_channel ch, const aux_uri *uri);

// 启动连接器（开始尝试连接）
aux_err aux_connector_start(aux_connector conn, int flags);

// 异步启动连接器
void aux_connector_start_async(aux_connector conn, int flags, aux_future *fut);

// 关闭连接器
aux_err aux_connector_close(aux_connector conn);

// 获取连接器 ID
int aux_connector_id(aux_connector conn);

// 获取连接器的 URL
aux_err aux_connector_get_url(aux_connector conn, const aux_uri **url);

/* ---- Connector 选项 ---- */

aux_err aux_connector_set_bool(aux_connector conn, const char *key, bool val);
aux_err aux_connector_set_int(aux_connector conn, const char *key, int val);
aux_err aux_connector_set_size(aux_connector conn, const char *key, size_t val);
aux_err aux_connector_set_duration(aux_connector conn, const char *key, aux_duration val);
aux_err aux_connector_set_string(aux_connector conn, const char *key, const char *val);
aux_err aux_connector_set_addr(aux_connector conn, const char *key, const aux_net_addr *addr);

aux_err aux_connector_get_bool(aux_connector conn, const char *key, bool *val);
aux_err aux_connector_get_int(aux_connector conn, const char *key, int *val);
aux_err aux_connector_get_size(aux_connector conn, const char *key, size_t *val);
aux_err aux_connector_get_duration(aux_connector conn, const char *key, aux_duration *val);
aux_err aux_connector_get_string(aux_connector conn, const char *key, const char **val);
aux_err aux_connector_get_addr(aux_connector conn, const char *key, aux_net_addr *addr);

/* ================================================================
 * Acceptor（接收器）- 被动接受连接
 * ================================================================ */

// 创建接收器（不立即启动）
aux_err aux_acceptor_create(
    aux_acceptor *acc, aux_channel ch, const char *addr);

// 创建接收器（通过 URI）
aux_err aux_acceptor_create_by_uri(
    aux_acceptor *acc, aux_channel ch, const aux_uri *uri);

// 启动接收器（开始监听）
aux_err aux_acceptor_start(aux_acceptor acc, int flags);

// 关闭接收器
aux_err aux_acceptor_close(aux_acceptor acc);

// 获取接收器 ID
int aux_acceptor_id(aux_acceptor acc);

// 获取接收器的 URL
aux_err aux_acceptor_get_url(aux_acceptor acc, const aux_uri **url);

/* ---- Acceptor 选项 ---- */

aux_err aux_acceptor_set_bool(aux_acceptor acc, const char *key, bool val);
aux_err aux_acceptor_set_int(aux_acceptor acc, const char *key, int val);
aux_err aux_acceptor_set_size(aux_acceptor acc, const char *key, size_t val);
aux_err aux_acceptor_set_duration(aux_acceptor acc, const char *key, aux_duration val);
aux_err aux_acceptor_set_string(aux_acceptor acc, const char *key, const char *val);

aux_err aux_acceptor_get_bool(aux_acceptor acc, const char *key, bool *val);
aux_err aux_acceptor_get_int(aux_acceptor acc, const char *key, int *val);
aux_err aux_acceptor_get_size(aux_acceptor acc, const char *key, size_t *val);
aux_err aux_acceptor_get_duration(aux_acceptor acc, const char *key, aux_duration *val);
aux_err aux_acceptor_get_string(aux_acceptor acc, const char *key, const char **val);

#ifdef __cplusplus
}
#endif

#endif /* AUX_CONNECTOR_H */
