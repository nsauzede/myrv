#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rv.h"

#define die()                                                                  \
  do {                                                                         \
    printf("DIE! %s:%s:%d\n", __func__, __FILE__, __LINE__);                   \
    if (ctx) {                                                                 \
      printf("PC=%08" PRIx32 " ", ctx->pc);                                    \
      rv_print_insn(ctx->last_insn);                                           \
    }                                                                          \
    exit(1);                                                                   \
  } while (0)

void rv_print_insn(uint32_t insn) {
  rv_insn i;
  i.insn = insn;
  printf("0x%08" PRIx32 " ", i.insn);
  printf("func7=%" PRIx8 " ", i.r.funct7);
  printf("rs2=%" PRIx32 " ", i.r.rs2);
  printf("rs1=%" PRIx32 " ", i.r.rs1);
  printf("funct3=%" PRIx8 " ", i.r.funct3);
  printf("rd=%" PRIx32 " ", i.r.rd);
  printf("opc=%" PRIx8 "\n", i.r.opc);
}

int rv_init(rv_ctx *ctx, rv_read32_cb rv_read32, rv_write32_cb rv_write32) {
  if (!ctx || !rv_read32 || !rv_write32) {
    return 1;
  }
  memset(ctx, 0, sizeof(*ctx));
  ctx->read32 = rv_read32;
  ctx->write32 = rv_write32;
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

int32_t rv_signext(int32_t val, int sbit) {
  int32_t sign = 0;
  int32_t mask = 1 << sbit;
  if (val & mask) {
    sign = -1 & ~(mask - 1);
  }
  return val | sign;
}

int rv_execute(rv_ctx *ctx) {
  if (!ctx) {
    return 1;
  }
  if (ctx->pc % 4) {
    return 2;
  }
  //   printf("%08" PRIx32 ": ", ctx->pc);
  if (rv_fetch(ctx)) {
    die();
    return 1;
  }

  rv_insn i;
  i.insn = ctx->last_insn;
  //   rv_print_insn(ctx->last_insn);

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
      ctx->x[i.i.rd] = ctx->x[i.i.rs1] + rv_signext(i.i.imm_11_0, 11);
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
#ifdef RV32M
    case RV_ADD_SUB_MUL:
#else
    case RV_ADD_SUB:
#endif
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
#ifdef RV32M
      case RV_MUL:
        printf("MUL rd=%s funct3=%" PRIx8 " rs1=%s rs2=%s", rv_rname(i.r.rd),
               i.r.funct3, rv_rname(i.r.rs1), rv_rname(i.r.rs2));
        ctx->x[i.r.rd] = ctx->x[i.r.rs1] * ctx->x[i.r.rs2];
        break;
#endif
      default:
        die();
        return 1;
      }
      break;
#ifdef RV32M
    case RV_XOR_DIV:
#else
    case RV_XOR:
#endif
      switch (i.r.funct7) {
#ifdef RV32M
      case RV_DIV:
        printf("DIV rd=%s funct3=%" PRIx8 " rs1=%s rs2=%s", rv_rname(i.r.rd),
               i.r.funct3, rv_rname(i.r.rs1), rv_rname(i.r.rs2));
        ctx->x[i.r.rd] = ctx->x[i.r.rs1] / ctx->x[i.r.rs2];
        break;
#endif
      default:
        die();
        return 1;
      }
      break;
#ifdef RV32M
    case RV_OR_REM:
#else
    case RV_OR:
#endif
      switch (i.r.funct7) {
#ifdef RV32M
      case RV_REM:
        printf("DIV rd=%s funct3=%" PRIx8 " rs1=%s rs2=%s", rv_rname(i.r.rd),
               i.r.funct3, rv_rname(i.r.rs1), rv_rname(i.r.rs2));
        ctx->x[i.r.rd] = ctx->x[i.r.rs1] % ctx->x[i.r.rs2];
        break;
#endif
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
    printf("LUI rd=%s imm31_12=%" PRIx32 "\n", rv_rname(i.u.rd), i.u.imm_31_12);
    ctx->x[i.u.rd] = i.u.imm_31_12 << 12;
    break;
  case RV_LOAD:
    printf("LOAD ");
    switch (i.r.funct3) {
    case RV_LW: {
      uint32_t imm = i.i.imm_11_0;
      printf("LW rd=%s funct3=%" PRIx8 " rs1=%s imm=%" PRIx32, rv_rname(i.i.rd),
             i.i.funct3, rv_rname(i.i.rs1), imm);
      ctx->x[i.i.rd] = ctx->read32(ctx->x[i.r.rs1] + imm);
      break;
    }
    default:
      die();
      return 1;
    }
    printf("\n");
    break;
  case RV_STORE:
    printf("STORE ");
    switch (i.r.funct3) {
    case RV_SW: {
      uint32_t imm = i.s.imm_4_0 + (i.s.imm_11_5 << 5);
      printf("SW rd=%s funct3=%" PRIx8 " rs1=%s imm=%" PRIx32 "\n",
             rv_rname(i.s.rs1), i.s.funct3, rv_rname(i.s.rs2), imm);
      // uint32_t addr=ctx->read32(ctx->x[i.r.rs1] + imm);
      ctx->write32(ctx->x[i.s.rs1] + imm, ctx->x[i.s.rs2]);
      break;
    }
    default:
      die();
      return 1;
    }
    break;
  case RV_AUIPC:
    printf("AUIPC rd=%s imm31_12=%" PRIx32 "\n", rv_rname(i.u.rd),
           i.u.imm_31_12);
    ctx->x[i.u.rd] = (i.u.imm_31_12 << 12) + ctx->pc - 4;
    break;

  case RV_JAL: {
    uint32_t imm = (i.j.imm_20 << 20) + (i.j.imm_19_12 << 12) +
                   (i.j.imm11 << 11) + (i.j.imm_10_1 << 1);
    printf("JAL rd=%s imm=%" PRIx32 "\n", rv_rname(i.j.rd), imm);
    ctx->x[i.j.rd] = ctx->pc;
    ctx->pc = imm + ctx->pc - 4;
    break;
  }

  case RV_JALR:
    printf("JALR rd=%s rs1=%s imm11_0=%" PRIx32 "\n", rv_rname(i.i.rd),
           rv_rname(i.i.rs1), i.i.imm_11_0);
    ctx->x[i.i.rd] = ctx->pc;
    ctx->pc = (i.i.imm_11_0 + ctx->x[i.i.rs1]) & ~1;
    break;

  case RV_SYSTEM:
    switch (i.sy.funct3) {
    case RV_PRIV:
      switch (i.sy.funct12) {
      case RV_EBREAK:
        printf("BREAK\n");
        return 1;
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
    break;
  default:
    die();
    return 1;
  }
  //   printf("ra=%08" PRIx32 "\n", ctx->ra);

  return 0;
}
