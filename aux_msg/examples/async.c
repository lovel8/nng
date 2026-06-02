/*
 * async.c - AUX 异步通信示例
 *
 * 演示 Future 的使用方式：回调模式 + 同步等待模式
 *
 * 编译: gcc -o async async.c -laux_msg
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aux_msg/aux_msg.h"

#define ADDR "tcp://127.0.0.1:19997"

// ================================================================
// 异步接收回调
// ================================================================

typedef struct {
    aux_future   fut;
    aux_channel  ch;
    bool         done;
} async_ctx;

static void
on_recv(void *arg)
{
    async_ctx *ctx = (async_ctx *)arg;

    aux_err result = aux_future_result(&ctx->fut);
    if (result != AUX_OK) {
        fprintf(stderr, "[ASYNC] 接收错误: %s\n", aux_strerror(result));
        ctx->done = true;
        return;
    }

    size_t count = aux_future_count(&ctx->fut);
    printf("[ASYNC] 收到 %zu 字节\n", count);

    // 继续接收下一条消息
    aux_future_reset(&ctx->fut);
    aux_channel_recv_async(ctx->ch, &ctx->fut);
}

// ================================================================
// 异步发布端
// ================================================================

static void
run_async_publisher(void)
{
    aux_channel pub;
    aux_err     rv;

    printf("[PUB] 打开异步发布者...\n");
    if ((rv = aux_publisher_open(&pub, ADDR)) != AUX_OK) {
        fprintf(stderr, "[PUB] 错误: %s\n", aux_strerror(rv));
        return;
    }

    aux_sleep(200);

    for (int i = 0; i < 5; i++) {
        char msg[64];
        snprintf(msg, sizeof(msg), "异步消息 #%d", i + 1);
        printf("[PUB] 发送: %s\n", msg);

        aux_channel_send(pub, msg, strlen(msg) + 1, 0);
        aux_sleep(200);
    }

    aux_channel_close(pub);
    printf("[PUB] 完成\n");
}

// ================================================================
// 异步订阅端（使用 Future + 回调）
// ================================================================

static void
run_async_subscriber(void)
{
    aux_channel sub;
    aux_err     rv;

    printf("[SUB] 打开异步订阅者...\n");
    if ((rv = aux_subscriber_open(&sub, ADDR)) != AUX_OK) {
        fprintf(stderr, "[SUB] 错误: %s\n", aux_strerror(rv));
        return;
    }

    aux_subscriber_subscribe_all(sub);

    async_ctx ctx = { .ch = sub, .done = false };

    // 分配 Future 并设置回调
    aux_future_alloc(&ctx.fut, on_recv, &ctx);

    // 启动首次异步接收
    aux_channel_recv_async(sub, &ctx.fut);

    // 等待一段时间让回调执行
    printf("[SUB] 等待异步消息...\n");
    aux_sleep(2000);

    // 停止并清理
    aux_future_stop(&ctx.fut);
    aux_future_free(&ctx.fut);
    aux_channel_close(sub);
    printf("[SUB] 完成\n");
}

// ================================================================
// 同步等待模式演示
// ================================================================

static void
run_sync_wait(void)
{
    printf("\n=== 同步等待模式演示 ===\n");

    aux_channel pub, sub;
    aux_err     rv;

    aux_publisher_open(&pub, "tcp://127.0.0.1:19996");
    aux_subscriber_open(&sub, "tcp://127.0.0.1:19996");
    aux_subscriber_subscribe_all(sub);
    aux_sleep(100);

    // 异步发送
    aux_future send_fut;
    aux_envelope env;
    aux_envelope_alloc(&env, 0);

    const char *msg = "Future消息";
    aux_envelope_body_append(&env, msg, strlen(msg) + 1);

    aux_future_alloc(&send_fut, NULL, NULL); // NULL 回调 = 同步等待模式
    aux_future_set_envelope(&send_fut, &env);
    aux_channel_send_async(pub, &send_fut);
    aux_future_wait(&send_fut); // 同步等待

    printf("[SYNC] 发送结果: %s\n",
        aux_future_result(&send_fut) == AUX_OK ? "成功" : "失败");

    // 异步接收
    aux_future recv_fut;
    aux_future_alloc(&recv_fut, NULL, NULL);
    aux_channel_recv_async(sub, &recv_fut);
    aux_future_wait(&recv_fut); // 同步等待

    if (aux_future_result(&recv_fut) == AUX_OK) {
        size_t count = aux_future_count(&recv_fut);
        printf("[SYNC] 收到 %zu 字节\n", count);
    }

    aux_future_free(&send_fut);
    aux_future_free(&recv_fut);
    aux_envelope_free(&env);
    aux_channel_close(pub);
    aux_channel_close(sub);
}

int
main(int argc, char **argv)
{
    aux_runtime_init(NULL);

    if (argc > 1 && strcmp(argv[1], "pub") == 0) {
        run_async_publisher();
    } else if (argc > 1 && strcmp(argv[1], "sub") == 0) {
        run_async_subscriber();
    } else if (argc > 1 && strcmp(argv[1], "sync") == 0) {
        run_sync_wait();
    } else {
        printf("用法: %s pub|sub|sync\n", argv[0]);
        printf("  pub  - 异步发布端\n");
        printf("  sub  - 异步订阅端（回调模式）\n");
        printf("  sync - 同步等待模式演示\n");
    }

    aux_runtime_fini();
    return 0;
}
