#include "em_gdbstub.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct gctx {
    pthread_t thr;
    rv_ctx *rctx;
    int port;
    int ss, cs;
    int to_thr[2];
    int from_thr[2];
} gs_ctx;

typedef enum {
    GS_STATUS,
} gs_cmd_t;

static void *gthread(void *arg) {
    gs_ctx *gctx = (gs_ctx *)arg;
    while (1) {
        int max = -1;
        fd_set rfds;
        FD_ZERO(&rfds);
        if (gctx->to_thr[0] != -1) {
            FD_SET(gctx->to_thr[0], &rfds);
            if (max < gctx->to_thr[0]) {
                max = gctx->to_thr[0];
            }
        }
        if (gctx->cs != -1) {
            FD_SET(gctx->cs, &rfds);
            if (max < gctx->cs) {
                max = gctx->cs;
            }
        }
        int n = select(max + 1, &rfds, 0, 0, 0);
        if (n == -1) {
            perror("select");
            exit(1);
        }
        if (n > 0) {
           printf("Something to read! %d\n", n);
            if (FD_ISSET(gctx->cs, &rfds)) {
                printf("Reading CS\n");
                int val;
                n = read(gctx->cs, &val, sizeof(val));
                if (!n) {
                    printf("CS hangup\n");
                    printf("Writing STATUS\n");
                    gstatus_t gs = GS_QUIT;
                    write(gctx->from_thr[1], &gs, sizeof(gs));
                    break;
                }
                printf("Got CS %d\n", val);
                printf("Writing STATUS\n");
                gstatus_t gs = GS_EXECUTE;
                write(gctx->from_thr[1], &gs, sizeof(gs));
            }
            if (FD_ISSET(gctx->to_thr[0], &rfds)) {
                printf("Reading CMD\n");
                gs_cmd_t cmd;
                read(gctx->to_thr[0], &cmd, sizeof(cmd));
                printf("Got CMD %d\n", cmd);
            }
        }
//        sleep(1);
    }
    return 0;
}

void *ginit(rv_ctx *rctx, int port) {
    gs_ctx * gctx = calloc(1, sizeof(gs_ctx));
    gctx->rctx = rctx;
    gctx->port = port;
    gctx->ss = socket(PF_INET, SOCK_STREAM, 0);
    int on = 1;
    setsockopt(gctx->ss, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(gctx->port);
    sa.sin_addr.s_addr = INADDR_ANY;
    bind(gctx->ss, (struct sockaddr *)&sa, sizeof(sa));
    listen(gctx->ss, 1);
    printf("Listening on port %d..\n", gctx->port);
    gctx->cs = accept(gctx->ss, 0, 0);
    if (gctx->cs == -1) {
        printf("Accept returned %d\n", gctx->cs);
        free(gctx);
        return 0;
    }
    printf("Accepted %d on %d\n", gctx->cs, gctx->port);
    pipe(gctx->to_thr);
    pipe(gctx->from_thr);
    if (pthread_create(&gctx->thr, 0, gthread, gctx)) {
        printf("Failed to create gthread\n");
        free(gctx);
        return 0;
    }
    printf("Created gthread\n");
    return gctx;
}

gstatus_t gstatus(struct gctx *gctx) {
    gstatus_t gs = GS_STALL;
    gs_cmd_t cmd = GS_STATUS;
    write(gctx->to_thr[1], &cmd, sizeof(cmd));
    read(gctx->from_thr[0], &gs, sizeof(gs));
    return gs;
}

void gcleanup(struct gctx *gctx) {
    if (gctx) {
        free(gctx);
    }
}
