#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rv.h"

#define die()                                                                  \
  do {                                                                         \
    printf("DIE! %s:%s:%d\n", __func__, __FILE__, __LINE__);                   \
    exit(1);                                                                   \
  } while (0)

int rv_init(rv_ctx *ctx, rv_read32_cb rv_read32, rv_write32_cb rv_write32) {
  if (!ctx || !rv_read32 || !rv_write32) {
    return 1;
  }
  memset(ctx, 0, sizeof(*ctx));
  ctx->read32 = rv_read32;
  return 0;
}

int rv_fetch(rv_ctx *ctx) {
  if (!ctx) {
    die();
    return 1;
  }
  ctx->last_insn = ctx->read32(ctx->pc);
  ctx->pc += 4;
  return 0;
}

char *rv_rname(uint8_t reg) {
  char *regs[] = {"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0",
                  "s1",   "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
                  "s2",   "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10",
                  "s11",  "t3", "t4", "t5", "t6", "??"};
  if (reg > sizeof(regs) / sizeof(regs[0]))
    reg = sizeof(regs) / sizeof(regs[0]);
  return regs[reg];
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
    die();
    return 1;
  }

  rv_insn i;
  i.insn = ctx->last_insn;
  printf("0x%08" PRIx32 " ", i.insn);
  printf("func7=%" PRIx8 " ", i.r.funct7);
  printf("rs2=%" PRIx32 " ", i.r.rs2);
  printf("rs1=%" PRIx32 " ", i.r.rs1);
  printf("funct3=%" PRIx8 " ", i.r.funct3);
  printf("rd=%" PRIx32 " ", i.r.rd);
  printf("opc=%" PRIx8 "\n", i.r.opc);

  switch (i.opc) {
  case RV_OP_IMM:
    printf("OP-IMM ");
    switch (i.i.funct3) {
    case RV_SLLI:
      switch (i.sh.imm_11_5) {
      case RV_S_LI:
        printf("SLLI rd=%s funct3=%" PRIx8 " rs1=%s imm4_0=%" PRIx32,
               rv_rname(i.sh.rd), i.sh.funct3, rv_rname(i.sh.rs1),
               i.sh.imm_4_0);
        ctx->x[i.sh.rd] = ctx->x[i.sh.rs1] << i.sh.imm_4_0;
        break;
      default:
        die();
        return 1;
      }
      break;
    case RV_ADDI:
      printf("ADDI rd=%s funct3=%" PRIx8 " rs1=%s imm11_0=%" PRIx32,
             rv_rname(i.i.rd), i.i.funct3, rv_rname(i.i.rs1), i.i.imm_11_0);
      ctx->x[i.i.rd] = ctx->x[i.i.rs1] + i.i.imm_11_0;
      break;
    default:
      die();
      return 1;
    }
    printf("\n");
    break;
  case RV_OP:
    printf("OP ");
    switch (i.r.funct3) {
    case RV_ADD_SUB:
      switch (i.r.funct7) {
      case RV_SUB:
        printf("SUB rd=%s funct3=%" PRIx8 " rs1=%s rs2=%s", rv_rname(i.r.rd),
               i.r.funct3, rv_rname(i.r.rs1), rv_rname(i.r.rs2));
        ctx->x[i.r.rd] = ctx->x[i.r.rs1] - ctx->x[i.r.rs2];
        break;
      case RV_ADD:
        printf("ADD rd=%s funct3=%" PRIx8 " rs1=%s rs2=%s", rv_rname(i.r.rd),
               i.r.funct3, rv_rname(i.r.rs1), rv_rname(i.r.rs2));
        ctx->x[i.r.rd] = ctx->x[i.r.rs1] + ctx->x[i.r.rs2];
        break;
      default:
        die();
        return 1;
      }
      break;
    default:
      die();
      return 1;
    }
    printf("\n");
    break;
  case RV_LUI:
    printf("LUI rd=%s imm31_12=%" PRIx32 "\n", rv_rname(i.laui.rd),
           i.laui.imm_31_12);
    ctx->x[i.laui.rd] = i.laui.imm_31_12 << 12;
    break;
  case RV_LOAD:
    printf("LOAD ");
    switch (i.r.funct3) {
    case RV_LW:
      printf("LW rd=%s funct3=%" PRIx8 " rs1=%s imm11_0=%" PRIx32,
             rv_rname(i.i.rd), i.i.funct3, rv_rname(i.i.rs1), i.i.imm_11_0);
      ctx->x[i.i.rd] = ctx->read32(ctx->x[i.r.rs1] + i.i.imm_11_0);
      break;
    default:
      die();
      return 1;
    }
    printf("\n");
    break;
  case RV_JALR:
    printf("JALR rd=%s rs1=%s\n", rv_rname(i.i.rd), rv_rname(i.i.rs1));
    break;
  case RV_SYSTEM:
    printf("BREAK\n");
    break;
  default:
    die();
    return 1;
  }
  return 0;
}
