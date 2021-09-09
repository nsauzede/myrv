#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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
  printf("%08" PRIx32 ": ", ctx->pc);
  if (rv_fetch(ctx)) {
    return 1;
  }
  rv_print_insn(ctx->last_insn);
  uint8_t opc = ctx->last_insn & 0x7f;
  uint8_t rd = (ctx->last_insn & 0xf80) >> 7;
  uint8_t funct3 = (ctx->last_insn & 0x7000) >> 12;
  uint8_t rs1 = (ctx->last_insn & 0xf8000) >> 15;
  uint8_t rs2 = (ctx->last_insn & 0x1f00000) >> 20;
  uint8_t funct7 = (ctx->last_insn & 0xfe000000) >> 25;
  switch (opc) {
  case 0x13:
    printf("OP-IMM");
    switch (funct3) {
    case 0x01:
      printf(" SLLI rd=%" PRIx8 " funct3=%" PRIx8 " rs1=%" PRIx8, rd, funct3, rs1);
      break;
    default:
      return 1;
    }
    printf("\n");
    break;
  case 0x33:
    printf("OP");
    switch (funct3) {
    case 0x00:
      switch (funct7) {
      case 0x20:
        printf(" SUB rd=%" PRIx8 " funct3=%" PRIx8 " rs1=%" PRIx8 " rs2=%" PRIx8, rd, funct3, rs1, rs2);
        break;
      default:
        return 1;
      }
      break;
    default:
      return 1;
    }
    printf("\n");
    break;
  case 0x67:
    printf("JALR rd=%" PRIx8 " rs1=%" PRIx8 "\n", rd, rs1);
    break;
  case 0x73:
    printf("BREAK\n");
    break;
  default:
    return 1;
  }
  return 0;
}
