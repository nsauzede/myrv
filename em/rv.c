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

char *rv_rname(uint8_t reg) {
  switch (reg) {
  case 0:
    return "zero";
  case 1:
    return "ra";
  case 10:
    return "a0";
  case 11:
    return "a1";
  case 12:
    return "a2";
  case 13:
    return "a3";
  case 14:
    return "a4";
  case 15:
    return "a5";
  case 16:
    return "a6";
  case 17:
    return "a7";
  default:
    return "??";
  }
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
  //   rv_print_insn(ctx->last_insn);
  uint8_t opc = ctx->last_insn & 0x7f;
  uint8_t rd = (ctx->last_insn & 0xf80) >> 7;
  uint8_t funct3 = (ctx->last_insn & 0x7000) >> 12;
  uint8_t rs1 = (ctx->last_insn & 0xf8000) >> 15;
  uint8_t rs2 = (ctx->last_insn & 0x1f00000) >> 20;
  uint8_t funct7 = (ctx->last_insn & 0xfe000000) >> 25;

  rv_insn i;
  i.insn = ctx->last_insn;
  printf("%" PRIx32 " ", i.insn);
  printf("func7=%" PRIx8 " ", i.r.funct7);
  printf("rs2=%" PRIx32 " ", i.r.rs2);
  printf("rs1=%" PRIx32 " ", i.r.rs1);
  printf("funct3=%" PRIx8 " ", i.r.funct3);
  printf("rd=%" PRIx32 " ", i.r.rd);
  printf("opc=%" PRIx8 "\n", i.r.opc);

  switch (i.opc) {
  case 0x13:
    printf("OP-IMM");
    switch (i.i.funct3) {
    case 0x01:
      printf(" SLLI rd=%s funct3=%" PRIx8 " rs1=%s", rv_rname(i.i.rd),
             i.i.funct3, rv_rname(i.i.rs1));
      break;
    default:
      return 1;
    }
    printf("\n");
    break;
  case 0x33:
    printf("OP");
    switch (i.r.funct3) {
    case 0x00:
      switch (i.r.funct7) {
      case 0x20:
        printf(" SUB rd=%s funct3=%" PRIx8 " rs1=%s rs2=%s", rv_rname(i.r.rd),
               i.r.funct3, rv_rname(i.r.rs1), rv_rname(i.r.rs2));
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
/*
JAL:  imm_20 imm_10_1 imm_11 imm_19_12 rd opc
JALR: imm_11_0 rs1 funct3 rd opc
*/
  case 0x67:
    printf("JALR rd=%s rs1=%s\n", rv_rname(i.i.rd), rv_rname(i.i.rs1));
    break;
  case 0x73:
    printf("BREAK\n");
    break;
  default:
    return 1;
  }
  return 0;
}
