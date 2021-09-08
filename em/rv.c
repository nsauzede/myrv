#include <string.h>

#include "rv.h"

int rv_init(rv_ctx *ctx) {
	if (!ctx) {
		return 1;
	}
	memset(ctx, 0, sizeof(*ctx));
	return 0;
}

int rv_execute(rv_ctx *ctx) {
	if (!ctx) {
		return 1;
	}
	ctx->pc++;
	return 0;
}
