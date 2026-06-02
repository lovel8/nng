/*
 * aux_connection.h - 连接实例（Connection）领域 API
 *
 * 装饰器模式封装 nng_pipe 操作。
 * DDD 概念：Connection（连接实例）= 已建立的点对点连接。
 *
 * Copyright (c) 2025 - Licensed under MIT
 */

#ifndef AUX_CONNECTION_H
#define AUX_CONNECTION_H

#include "aux_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * 生命周期
 * ================================================================ */

// 关闭连接
aux_err aux_connection_close(aux_connection conn);

// 获取连接 ID
int aux_connection_id(aux_connection conn);

// 获取连接所属的通道
aux_channel aux_connection_get_channel(aux_connection conn);

// 获取连接所属的连接器
aux_connector aux_connection_get_connector(aux_connection conn);

// 获取连接所属的接收器
aux_acceptor aux_connection_get_acceptor(aux_connection conn);

/* ================================================================
 * 地址信息
 * ================================================================ */

// 获取对端地址
aux_err aux_connection_peer_addr(aux_connection conn, aux_net_addr *addr);

// 获取本地地址
aux_err aux_connection_self_addr(aux_connection conn, aux_net_addr *addr);

/* ================================================================
 * 选项查询
 * ================================================================ */

aux_err aux_connection_get_bool(aux_connection conn, const char *key, bool *val);
aux_err aux_connection_get_int(aux_connection conn, const char *key, int *val);
aux_err aux_connection_get_size(aux_connection conn, const char *key, size_t *val);
aux_err aux_connection_get_duration(aux_connection conn, const char *key, aux_duration *val);
aux_err aux_connection_get_string(aux_connection conn, const char *key, const char **val);

#ifdef __cplusplus
}
#endif

#endif /* AUX_CONNECTION_H */
