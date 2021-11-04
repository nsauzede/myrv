#ifndef EM_GDBSTUB_H_
#define EM_GDBSTUB_H_

#include "rv.h"

typedef enum {
    GS_QUIT,
    GS_EXECUTE,
} gstatus_t;

struct gctx;

void *ginit(rv_ctx *ctx, int port);
gstatus_t gstatus(struct gctx *gctx);
void gcleanup(struct gctx *gctx);

#endif/*EM_GDBSTUB_H_*/
