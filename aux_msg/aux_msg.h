/*
 * aux_msg.h - AUX 消息通信库统一头文件
 *
 * 基于装饰器模式 + DDD 领域驱动设计对底层通信库的二次封装。
 * 底层库可替换，对外接口保持稳定。
 *
 * 所有导出接口以 aux_ 开头，采用统一的领域语言：
 *
 *   Channel（通道）     → 封装 socket
 *   Connector（连接器）  → 封装 dialer
 *   Acceptor（接收器）   → 封装 listener
 *   Connection（连接）   → 封装 pipe
 *   Session（会话）      → 封装 ctx
 *   Envelope（信封）     → 封装 msg
 *   Future（异步句柄）   → 封装 aio
 *   ByteStream（字节流） → 封装 stream
 *
 * 特性：
 *   - 装饰器模式：不修改底层库源码，可随时替换底层实现
 *   - DDD 统一语言：屏蔽底层网络术语，使用业务领域概念
 *   - 所有接口以 aux_ 开头，避免符号冲突
 *   - 错误码语义化：AUX_OK / AUX_ERR_TIMEOUT / ...
 *
 * 使用方法：
 *   1. #include "aux_msg/aux_msg.h"
 *   2. 链接 aux_msg 库
 *   3. 编译时确保底层库头文件路径可用
 *
 * Copyright (c) 2025 - Licensed under MIT
 */

#ifndef AUX_MSG_H
#define AUX_MSG_H

#include "interface/aux_types.h"
#include "interface/aux_runtime.h"
#include "interface/aux_channel.h"
#include "interface/aux_connector.h"
#include "interface/aux_connection.h"
#include "interface/aux_envelope.h"
#include "interface/aux_future.h"
#include "interface/aux_session.h"
#include "interface/aux_subscriber.h"

/*
 * ================================================================
 * 使用示例
 * ================================================================
 *
 * // --- 发布/订阅 ---
 *
 *   aux_runtime_init(NULL);
 *
 *   // 发布端
 *   aux_channel pub;
 *   aux_publisher_open(&pub, "tcp://\*:5555");
 *   aux_channel_send(pub, "hello", 5, 0);
 *   aux_channel_close(pub);
 *
 *   // 订阅端
 *   aux_channel sub;
 *   aux_subscriber_open(&sub, "tcp://localhost:5555");
 *   aux_subscriber_subscribe_all(sub);
 *   char buf[1024]; size_t sz = sizeof(buf);
 *   aux_channel_recv(sub, buf, &sz, 0);
 *   aux_channel_close(sub);
 *
 *   aux_runtime_fini();
 *
 * // --- 请求/回复 ---
 *
 *   // 服务端
 *   aux_channel rep;
 *   aux_replier_open(&rep, "tcp://\*:5556");
 *   char buf[1024]; size_t sz = sizeof(buf);
 *   aux_channel_recv(rep, buf, &sz, 0);
 *   aux_channel_send(rep, "world", 5, 0);
 *   aux_channel_close(rep);
 *
 *   // 客户端
 *   aux_channel req;
 *   aux_requester_open(&req, "tcp://localhost:5556");
 *   aux_channel_send(req, "hello", 5, 0);
 *   sz = sizeof(buf);
 *   aux_channel_recv(req, buf, &sz, 0);
 *   aux_channel_close(req);
 *
 * // --- 异步操作 ---
 *
 *   void on_recv(void *arg) {
 *       aux_future *fut = (aux_future *)arg;
 *       if (aux_future_result(fut) == AUX_OK) {
 *           size_t count = aux_future_count(fut);
 *           // 处理数据...
 *       }
 *   }
 *
 *   aux_future fut;
 *   aux_future_alloc(&fut, on_recv, &fut);
 *   aux_channel_recv_async(ch, &fut);
 *
 */

#endif /* AUX_MSG_H */
