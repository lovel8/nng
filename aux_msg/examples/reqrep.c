/*
 * reqrep.c - AUX 请求/回复示例
 *
 * 编译: gcc -o reqrep reqrep.c -laux_msg
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aux_msg/aux_msg.h"

#define ADDR "tcp://127.0.0.1:19998"

static void
run_replier(void)
{
    aux_channel rep;
    aux_err     rv;

    printf("[REP] 打开回复者通道...\n");
    if ((rv = aux_replier_open(&rep, ADDR)) != AUX_OK) {
        fprintf(stderr, "[REP] 错误: %s\n", aux_strerror(rv));
        return;
    }

    for (int i = 0; i < 3; i++) {
        char buf[128];
        size_t sz = sizeof(buf);

        printf("[REP] 等待请求...\n");
        rv = aux_channel_recv(rep, buf, &sz, 0);
        if (rv != AUX_OK) {
            fprintf(stderr, "[REP] 接收失败: %s\n", aux_strerror(rv));
            break;
        }
        printf("[REP] 收到请求: %s\n", buf);

        const char *reply = "回复: 已收到";
        rv = aux_channel_send(rep, reply, strlen(reply) + 1, 0);
        if (rv != AUX_OK) {
            fprintf(stderr, "[REP] 发送失败: %s\n", aux_strerror(rv));
            break;
        }
        printf("[REP] 已发送回复\n");
    }

    aux_channel_close(rep);
    printf("[REP] 完成\n");
}

static void
run_requester(void)
{
    aux_channel req;
    aux_err     rv;

    printf("[REQ] 打开请求者通道...\n");
    if ((rv = aux_requester_open(&req, ADDR)) != AUX_OK) {
        fprintf(stderr, "[REQ] 错误: %s\n", aux_strerror(rv));
        return;
    }

    for (int i = 0; i < 3; i++) {
        char msg[64];
        snprintf(msg, sizeof(msg), "请求 #%d", i + 1);
        printf("[REQ] 发送: %s\n", msg);

        rv = aux_channel_send(req, msg, strlen(msg) + 1, 0);
        if (rv != AUX_OK) {
            fprintf(stderr, "[REQ] 发送失败: %s\n", aux_strerror(rv));
            break;
        }

        char buf[128];
        size_t sz = sizeof(buf);
        rv = aux_channel_recv(req, buf, &sz, 0);
        if (rv != AUX_OK) {
            fprintf(stderr, "[REQ] 接收失败: %s\n", aux_strerror(rv));
            break;
        }
        printf("[REQ] 收到回复: %s\n", buf);
    }

    aux_channel_close(req);
    printf("[REQ] 完成\n");
}

int
main(int argc, char **argv)
{
    aux_runtime_init(NULL);

    if (argc > 1 && strcmp(argv[1], "rep") == 0) {
        run_replier();
    } else if (argc > 1 && strcmp(argv[1], "req") == 0) {
        run_requester();
    } else {
        printf("用法: %s rep|req\n", argv[0]);
        printf("  先启动 rep 端，再启动 req 端\n");
    }

    aux_runtime_fini();
    return 0;
}
