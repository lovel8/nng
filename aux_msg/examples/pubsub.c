/*
 * pubsub.c - AUX 发布/订阅示例
 *
 * 编译: gcc -o pubsub pubsub.c -laux_msg
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "aux_msg/aux_msg.h"

#define ADDR "tcp://127.0.0.1:19999"

static void
run_publisher(void)
{
    aux_channel pub;
    aux_err     rv;

    printf("[PUB] 打开发布者通道...\n");
    if ((rv = aux_publisher_open(&pub, ADDR)) != AUX_OK) {
        fprintf(stderr, "[PUB] 错误: %s\n", aux_strerror(rv));
        return;
    }

    // 给订阅者一点时间连接
    aux_sleep(200);

    for (int i = 0; i < 5; i++) {
        char msg[64];
        snprintf(msg, sizeof(msg), "消息 #%d", i + 1);
        printf("[PUB] 发送: %s\n", msg);

        rv = aux_channel_send(pub, msg, strlen(msg) + 1, 0);
        if (rv != AUX_OK) {
            fprintf(stderr, "[PUB] 发送失败: %s\n", aux_strerror(rv));
        }
        aux_sleep(100);
    }

    aux_channel_close(pub);
    printf("[PUB] 完成\n");
}

static void
run_subscriber(void)
{
    aux_channel sub;
    aux_err     rv;

    printf("[SUB] 打开订阅者通道...\n");
    if ((rv = aux_subscriber_open(&sub, ADDR)) != AUX_OK) {
        fprintf(stderr, "[SUB] 错误: %s\n", aux_strerror(rv));
        return;
    }

    // 订阅所有消息
    aux_subscriber_subscribe_all(sub);
    printf("[SUB] 已订阅所有消息\n");

    for (int i = 0; i < 5; i++) {
        char buf[128];
        size_t sz = sizeof(buf);

        rv = aux_channel_recv(sub, buf, &sz, 0);
        if (rv == AUX_OK) {
            printf("[SUB] 收到: %s (%zu 字节)\n", buf, sz);
        } else {
            fprintf(stderr, "[SUB] 接收失败: %s\n", aux_strerror(rv));
        }
    }

    aux_channel_close(sub);
    printf("[SUB] 完成\n");
}

int
main(int argc, char **argv)
{
    aux_runtime_init(NULL);

    if (argc > 1 && strcmp(argv[1], "pub") == 0) {
        run_publisher();
    } else if (argc > 1 && strcmp(argv[1], "sub") == 0) {
        run_subscriber();
    } else {
        printf("用法: %s pub|sub\n", argv[0]);
        printf("  先启动 sub 端，再启动 pub 端\n");
    }

    aux_runtime_fini();
    return 0;
}
