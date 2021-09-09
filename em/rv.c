#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "rv.h"

int rv_init(rv_ctx *ctx) {
  if (!ctx) {
    return 1;
  }
  memset(ctx, 0, sizeof(*ctx));
  return 0;
}

int rv_fetch(rv_ctx *ctx) {
  if (!ctx) {
    return 1;
  }
  if (!ctx->read32) {
    return 1;
  }
  ctx->last_insn = ctx->read32(ctx->pc);
  ctx->pc += 4;
  return 0;
}

int rv_print_insn(uint32_t insn) {
	uint8_t opc = insn & 0x7f;
	printf("INSN=0x%08" PRIx32 " OPC=0x%02" PRIx8 "\n", insn, opc);
}

int rv_execute(rv_ctx *ctx) {
  if (!ctx) {
    return 1;
  }
  if (ctx->pc % 4) {
    return 2;
  }
  if (rv_fetch(ctx)) {
    return 1;
  }
  rv_print_insn(ctx->last_insn);
  return 0;
}
