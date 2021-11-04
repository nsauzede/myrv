#include "em_gdbstub.h"

#include <malloc.h>

typedef struct gctx {
    rv_ctx *rctx;
} gctx;

void *ginit(rv_ctx *rctx, int port) {
    gctx * ctx = calloc(1, sizeof(gctx));
    ctx->rctx = rctx;
    return ctx;
}

gstatus_t gstatus(struct gctx *gctx) {
    return GS_EXECUTE;
}

void gcleanup(struct gctx *gctx) {
    if (gctx) {
        free(gctx);
    }
}
