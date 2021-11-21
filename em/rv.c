#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "rv.h"

#ifdef HAVE_GDBSTUB
#include <arpa/inet.h>
#include "librspd.h"
#endif

#define die()                                                                  \
  do {                                                                         \
    printf("DIE! %s:%s:%d\n", __func__, __FILE__, __LINE__);                   \
    if (ctx) {                                                                 \
      rv_print_insn(ctx);                                                      \
    }                                                                          \
    exit(1);                                                                   \
  } while (0)

static int g_log = 0;
#define log_printf(l, ...)                                                     \
  do {                                                                         \
    if (l <= g_log) {                                                          \
      printf(__VA_ARGS__);                                                     \
    }                                                                          \
  } while (0)

int
rv_set_log(rv_ctx* ctx, int log)
{
  g_log = log;
  log_printf(1, "setting log to %d\n", log);
  return 0;
}

void
rv_print_insn(rv_ctx* ctx)
{
  printf("PC 0x%08" PRIx32 " ", ctx->pc);
  rv_insn i;
  i.insn = ctx->last_insn;
  printf("0x%08" PRIx32 " ", i.insn);
  printf("func7=%" PRIx8 " ", i.r.funct7);
  printf("rs2=%" PRIx32 " ", i.r.rs2);
  printf("rs1=%" PRIx32 " ", i.r.rs1);
  printf("funct3=%" PRIx8 " ", i.r.funct3);
  printf("rd=%" PRIx32 " ", i.r.rd);
  printf("opc=%" PRIx8 "\n", i.r.opc);
}

static uint32_t
rv_read(rv_ctx* ctx, void* dest, uint32_t addr, uint32_t len)
{
  return ctx->read(dest, addr, len);
}

static uint8_t
rv_read8(rv_ctx* ctx, uint32_t addr)
{
  uint8_t val = 0;
  rv_read(ctx, &val, addr, sizeof(val));
  return val;
}

static uint16_t
rv_read16(rv_ctx* ctx, uint32_t addr)
{
  uint16_t val = 0;
  rv_read(ctx, &val, addr, sizeof(val));
  return val;
}

static uint32_t
rv_read32(rv_ctx* ctx, uint32_t addr)
{
  uint32_t val = 0;
  rv_read(ctx, &val, addr, sizeof(val));
  return val;
}

static uint32_t
rv_write8(rv_ctx* ctx, uint32_t addr, uint8_t val)
{
  return ctx->write(&val, addr, sizeof(val));
}

static uint32_t
rv_write16(rv_ctx* ctx, uint32_t addr, uint16_t val)
{
  return ctx->write(&val, addr, sizeof(val));
}

static uint32_t
rv_write32(rv_ctx* ctx, uint32_t addr, uint32_t val)
{
  return ctx->write(&val, addr, sizeof(val));
}

int
rv_fetch(rv_ctx* ctx)
{
  if (!ctx) {
    die();
    return 1;
  }
  ctx->last_insn = rv_read32(ctx, ctx->pc);
  ctx->pc_next = ctx->pc + 4;
  return 0;
}

char*
rv_rname(uint8_t reg)
{
  char* rv_rnames[] = { "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0",
                        "s1",   "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
                        "s2",   "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10",
                        "s11",  "t3", "t4", "t5", "t6", "pc", "??" };
  if (reg > sizeof(rv_rnames) / sizeof(rv_rnames[0]))
    reg = sizeof(rv_rnames) / sizeof(rv_rnames[0]);
  return rv_rnames[reg];
}

void
rv_print_regs(rv_ctx* ctx)
{
  for (int i = 1; i < RV_REGS; i++) {
    printf("%-15s0x%-8" PRIx32 "\t%-8" PRId32 "\n",
           rv_rname(i),
           ctx->x[i],
           ctx->x[i]);
  }
}

int64_t
rv_signext64(int64_t val, int sbit)
{
  int64_t sign = 0;
  int64_t mask = 1 << sbit;
  if (val & mask) {
    sign = -1 & ~(mask - 1);
  }
  return val | sign;
}

int32_t
rv_signext(int32_t val, int sbit)
{
  int32_t sign = 0;
  int32_t mask = 1 << sbit;
  if (val & mask) {
    sign = -1 & ~(mask - 1);
  }
  return val | sign;
}

int
rv_step(rv_ctx* ctx)
{
  int ret = 0;
  if (!ctx) {
    return 1;
  }
  if (ctx->pc % 4) {
    return 2;
  }
  if (rv_fetch(ctx)) {
    die();
    return 1;
  }

  rv_insn i;
  i.insn = ctx->last_insn;
  if (g_log >= 2) {
    rv_print_insn(ctx);
  } else {
    if (g_log >= 1) {
      rv_print_regs(ctx);
    }
    static int counter = 0;
    log_printf(
      1, "%d PC 0x%08" PRIx32 " 0x%08" PRIx32 " ", counter++, ctx->pc, i.insn);
  }

  switch (i.opc) {
    case RV_OP_IMM:
      log_printf(1, "OP-IMM ");
      switch (i.i.funct3) {
        case RV_SLLI:
          switch (i.sh.imm_11_5) {
            case RV_S_LI:
              log_printf(1,
                         "SLLI rd=%s funct3=%" PRIx8 " rs1=%s imm4_0=%" PRIx32,
                         rv_rname(i.sh.rd),
                         i.sh.funct3,
                         rv_rname(i.sh.rs1),
                         i.sh.imm_4_0);
              if (i.sh.rd)
                ctx->x[i.sh.rd] = ctx->x[i.sh.rs1] << i.sh.imm_4_0;
              break;
            default:
              die();
              return 1;
          }
          break;
        case RV_SR_I:
          switch (i.sh.imm_11_5) {
            case RV_S_LI:
              log_printf(1,
                         "SRLI rd=%s funct3=%" PRIx8 " rs1=%s imm4_0=%" PRIx32,
                         rv_rname(i.sh.rd),
                         i.sh.funct3,
                         rv_rname(i.sh.rs1),
                         i.sh.imm_4_0);
              if (i.sh.rd)
                ctx->x[i.sh.rd] = ctx->x[i.sh.rs1] >> i.sh.imm_4_0;
              break;
            case RV_S_AI:
              log_printf(1,
                         "SRAI rd=%s funct3=%" PRIx8 " rs1=%s imm4_0=%" PRIx32,
                         rv_rname(i.sh.rd),
                         i.sh.funct3,
                         rv_rname(i.sh.rs1),
                         i.sh.imm_4_0);
              if (i.sh.rd)
                ctx->x[i.sh.rd] = rv_signext(ctx->x[i.sh.rs1] >> i.sh.imm_4_0,
                                             32 - i.sh.imm_4_0 - 1);
              break;
            default:
              die();
              return 1;
          }
          break;
        case RV_ADDI:
          log_printf(1,
                     "ADDI rd=%s funct3=%" PRIx8 " rs1=%s imm11_0=%" PRIx32,
                     rv_rname(i.i.rd),
                     i.i.funct3,
                     rv_rname(i.i.rs1),
                     i.i.imm_11_0);
          if (i.i.rd)
            ctx->x[i.i.rd] = ctx->x[i.i.rs1] + rv_signext(i.i.imm_11_0, 11);
          break;
        case RV_SLTIU:
          log_printf(1,
                     "SLTIU rd=%s funct3=%" PRIx8 " rs1=%s imm11_0=%" PRIx32,
                     rv_rname(i.i.rd),
                     i.i.funct3,
                     rv_rname(i.i.rs1),
                     i.i.imm_11_0);
          if (i.i.rd)
            ctx->x[i.i.rd] =
              ctx->x[i.i.rs1] < (uint32_t)rv_signext(i.i.imm_11_0, 11) ? 1 : 0;
          break;
        case RV_XORI:
          log_printf(1,
                     "XORI rd=%s funct3=%" PRIx8 " rs1=%s imm11_0=%" PRIx32,
                     rv_rname(i.i.rd),
                     i.i.funct3,
                     rv_rname(i.i.rs1),
                     i.i.imm_11_0);
          if (i.i.rd)
            ctx->x[i.i.rd] = ctx->x[i.i.rs1] ^ rv_signext(i.i.imm_11_0, 11);
          break;
        case RV_ORI:
          log_printf(1,
                     "ORI rd=%s funct3=%" PRIx8 " rs1=%s imm11_0=%" PRIx32,
                     rv_rname(i.i.rd),
                     i.i.funct3,
                     rv_rname(i.i.rs1),
                     i.i.imm_11_0);
          if (i.i.rd)
            ctx->x[i.i.rd] = ctx->x[i.i.rs1] | rv_signext(i.i.imm_11_0, 11);
          break;
        case RV_ANDI:
          log_printf(1,
                     "ANDI rd=%s funct3=%" PRIx8 " rs1=%s imm11_0=%" PRIx32,
                     rv_rname(i.i.rd),
                     i.i.funct3,
                     rv_rname(i.i.rs1),
                     i.i.imm_11_0);
          if (i.i.rd)
            ctx->x[i.i.rd] = ctx->x[i.i.rs1] & rv_signext(i.i.imm_11_0, 11);
          break;
        default:
          die();
          return 1;
      }
      log_printf(1, "\n");
      break;
    case RV_OP:
      log_printf(1, "OP ");
      switch (i.r.funct3) {
#ifdef RV32M
        case RV_ADD_SUB_MUL:
#else
        case RV_ADD_SUB:
#endif
          switch (i.r.funct7) {
            case RV_SUB:
              log_printf(1,
                         "SUB rd=%s funct3=%" PRIx8 " rs1=%s rs2=%s",
                         rv_rname(i.r.rd),
                         i.r.funct3,
                         rv_rname(i.r.rs1),
                         rv_rname(i.r.rs2));
              if (i.r.rd)
                ctx->x[i.r.rd] = ctx->x[i.r.rs1] - ctx->x[i.r.rs2];
              break;
            case RV_ADD:
              log_printf(1,
                         "ADD rd=%s funct3=%" PRIx8 " rs1=%s rs2=%s",
                         rv_rname(i.r.rd),
                         i.r.funct3,
                         rv_rname(i.r.rs1),
                         rv_rname(i.r.rs2));
              if (i.r.rd)
                ctx->x[i.r.rd] = ctx->x[i.r.rs1] + ctx->x[i.r.rs2];
              break;
#ifdef RV32M
            case RV_MUL:
              log_printf(1,
                         "MUL rd=%s funct3=%" PRIx8 " rs1=%s rs2=%s",
                         rv_rname(i.r.rd),
                         i.r.funct3,
                         rv_rname(i.r.rs1),
                         rv_rname(i.r.rs2));
              if (i.r.rd)
                ctx->x[i.r.rd] = ctx->x[i.r.rs1] * ctx->x[i.r.rs2];
              break;
#endif
            default:
              die();
              return 1;
          }
          break;
#ifdef RV32M
        case RV_SLL_MULH:
#else
        case RV_SLL:
#endif
          switch (i.r.funct7) {
            case 0x00:
              log_printf(1,
                         "SLL rd=%s funct3=%" PRIx8 " rs1=%s rs2=%s",
                         rv_rname(i.r.rd),
                         i.r.funct3,
                         rv_rname(i.r.rs1),
                         rv_rname(i.r.rs2));
              if (i.r.rd)
                ctx->x[i.r.rd] = ctx->x[i.r.rs1] << (ctx->x[i.r.rs2] & 0x1f);
              break;
#ifdef RV32M
            case RV_MULH:
              log_printf(1,
                         "MULH rd=%s funct3=%" PRIx8 " rs1=%s rs2=%s",
                         rv_rname(i.r.rd),
                         i.r.funct3,
                         rv_rname(i.r.rs1),
                         rv_rname(i.r.rs2));
              if (i.r.rd) {
                //          int64_t rs1 = rv_signext64(ctx->x[i.r.rs1], 32);
                int64_t rs2 = rv_signext64(ctx->x[i.r.rs2], 32);
                int64_t rs1 = ctx->x[i.r.rs1];
                //          int64_t rs2 = ctx->x[i.r.rs2];
                log_printf(1, "RS1=%" PRIx64 " RS2=%" PRIx64 "\n", rs1, rs2);
                ctx->x[i.r.rd] = (rs1 * rs2) >> 32;
              }
              break;
#endif
            default:
              die();
              return 1;
          }
          break;
#ifdef RV32M
        case RV_SLTU_MULHU:
#else
        case RV_SLTU:
#endif
          switch (i.r.funct7) {
            case 0x00:
              log_printf(1,
                         "SLTU rd=%s funct3=%" PRIx8 " rs1=%s rs2=%s",
                         rv_rname(i.r.rd),
                         i.r.funct3,
                         rv_rname(i.r.rs1),
                         rv_rname(i.r.rs2));
              if (i.r.rd)
                ctx->x[i.r.rd] = ctx->x[i.r.rs1] < ctx->x[i.r.rs2] ? 1 : 0;
              break;
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
            case 0x00:
              log_printf(1,
                         "XOR rd=%s funct3=%" PRIx8 " rs1=%s rs2=%s",
                         rv_rname(i.r.rd),
                         i.r.funct3,
                         rv_rname(i.r.rs1),
                         rv_rname(i.r.rs2));
              if (i.r.rd)
                ctx->x[i.r.rd] = ctx->x[i.r.rs1] ^ ctx->x[i.r.rs2];
              break;
#ifdef RV32M
            case RV_DIV:
              log_printf(1,
                         "DIV rd=%s funct3=%" PRIx8 " rs1=%s rs2=%s",
                         rv_rname(i.r.rd),
                         i.r.funct3,
                         rv_rname(i.r.rs1),
                         rv_rname(i.r.rs2));
              if (i.r.rd)
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
            case 0x00:
              log_printf(1,
                         "OR rd=%s funct3=%" PRIx8 " rs1=%s rs2=%s",
                         rv_rname(i.r.rd),
                         i.r.funct3,
                         rv_rname(i.r.rs1),
                         rv_rname(i.r.rs2));
              if (i.r.rd)
                ctx->x[i.r.rd] = ctx->x[i.r.rs1] | ctx->x[i.r.rs2];
              break;
#ifdef RV32M
            case RV_REM:
              log_printf(1,
                         "REM rd=%s funct3=%" PRIx8 " rs1=%s rs2=%s",
                         rv_rname(i.r.rd),
                         i.r.funct3,
                         rv_rname(i.r.rs1),
                         rv_rname(i.r.rs2));
              if (i.r.rd)
                ctx->x[i.r.rd] = ctx->x[i.r.rs1] % ctx->x[i.r.rs2];
              break;
#endif
            default:
              die();
              return 1;
          }
          break;
#ifdef RV32M
        case RV_AND_REMU:
#else
        case RV_AND:
#endif
          switch (i.r.funct7) {
            case 0x00:
              log_printf(1,
                         "AND rd=%s funct3=%" PRIx8 " rs1=%s rs2=%s",
                         rv_rname(i.r.rd),
                         i.r.funct3,
                         rv_rname(i.r.rs1),
                         rv_rname(i.r.rs2));
              if (i.r.rd)
                ctx->x[i.r.rd] = ctx->x[i.r.rs1] & ctx->x[i.r.rs2];
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
      log_printf(1, "\n");
      break;
    case RV_LUI:
      log_printf(
        1, "LUI rd=%s imm31_12=%" PRIx32 "\n", rv_rname(i.u.rd), i.u.imm_31_12);
      if (i.u.rd)
        ctx->x[i.u.rd] = i.u.imm_31_12 << 12;
      break;
    case RV_LOAD:
      log_printf(1, "LOAD ");
      switch (i.r.funct3) {
        case RV_LBU: {
          uint32_t imm = i.i.imm_11_0;
          log_printf(1,
                     "LBU rd=%s funct3=%" PRIx8 " rs1=%s imm=%" PRIx32,
                     rv_rname(i.i.rd),
                     i.i.funct3,
                     rv_rname(i.i.rs1),
                     imm);
          if (i.i.rd) {
            //        printf("SRC=%" PRIx32 "\n", ctx->x[i.i.rs1] +
            //        rv_signext(imm, 11));
            ctx->x[i.i.rd] =
              rv_read8(ctx, ctx->x[i.i.rs1] + rv_signext(imm, 11));
          }
          break;
        }
        case RV_LH: {
          uint32_t imm = i.i.imm_11_0;
          log_printf(1,
                     "LH rd=%s funct3=%" PRIx8 " rs1=%s imm=%" PRIx32,
                     rv_rname(i.i.rd),
                     i.i.funct3,
                     rv_rname(i.i.rs1),
                     imm);
          if (i.i.rd)
            ctx->x[i.i.rd] = rv_signext(
              rv_read16(ctx, ctx->x[i.i.rs1] + rv_signext(imm, 11)), 15);
          break;
        }
        case RV_LHU: {
          uint32_t imm = i.i.imm_11_0;
          log_printf(1,
                     "LHU rd=%s funct3=%" PRIx8 " rs1=%s imm=%" PRIx32,
                     rv_rname(i.i.rd),
                     i.i.funct3,
                     rv_rname(i.i.rs1),
                     imm);
          if (i.i.rd)
            ctx->x[i.i.rd] =
              rv_read16(ctx, ctx->x[i.i.rs1] + rv_signext(imm, 11));
          break;
        }
        case RV_LW: {
          uint32_t imm = i.i.imm_11_0;
          log_printf(1,
                     "LW rd=%s funct3=%" PRIx8 " rs1=%s imm=%" PRIx32,
                     rv_rname(i.i.rd),
                     i.i.funct3,
                     rv_rname(i.i.rs1),
                     imm);
          if (i.i.rd)
            ctx->x[i.i.rd] =
              rv_read32(ctx, ctx->x[i.i.rs1] + rv_signext(imm, 11));
          break;
        }
        default:
          die();
          return 1;
      }
      log_printf(1, "\n");
      break;

    case RV_BRANCH:
      log_printf(1, "BRANCH ");
      switch (i.r.funct3) {
        case RV_BLT: {
          uint32_t imm = (i.b.imm_12 << 12) + (i.b.imm_11 << 11) +
                         (i.b.imm_10_5 << 5) + (i.b.imm_4_1 << 1);
          log_printf(1,
                     "BLT rs1=%s rs2=%s imm=%" PRIx32 "\n",
                     rv_rname(i.b.rs1),
                     rv_rname(i.b.rs2),
                     imm);
          if ((int32_t)(ctx->x[i.b.rs1]) < (int32_t)(ctx->x[i.b.rs2])) {
            ctx->pc_next = ctx->pc + rv_signext(imm, 12);
          }
          break;
        }
        case RV_BLTU: {
          uint32_t imm = (i.b.imm_12 << 12) + (i.b.imm_11 << 11) +
                         (i.b.imm_10_5 << 5) + (i.b.imm_4_1 << 1);
          log_printf(1,
                     "BLTu rs1=%s rs2=%s imm=%" PRIx32 "\n",
                     rv_rname(i.b.rs1),
                     rv_rname(i.b.rs2),
                     imm);
          if (ctx->x[i.b.rs1] < ctx->x[i.b.rs2]) {
            ctx->pc_next = ctx->pc + rv_signext(imm, 12);
          }
          break;
        }
        case RV_BEQ: {
          uint32_t imm = (i.b.imm_12 << 12) + (i.b.imm_11 << 11) +
                         (i.b.imm_10_5 << 5) + (i.b.imm_4_1 << 1);
          log_printf(1,
                     "BEQ rs1=%s=%" PRIx32 " rs2=%s=%" PRIx32 " imm=%" PRIx32
                     "\n",
                     rv_rname(i.b.rs1),
                     ctx->x[i.b.rs1],
                     rv_rname(i.b.rs2),
                     ctx->x[i.b.rs2],
                     imm);
          if (ctx->x[i.b.rs1] == ctx->x[i.b.rs2]) {
            ctx->pc_next = ctx->pc + rv_signext(imm, 12);
          }
          break;
        }
        case RV_BNE: {
          uint32_t imm = (i.b.imm_12 << 12) + (i.b.imm_11 << 11) +
                         (i.b.imm_10_5 << 5) + (i.b.imm_4_1 << 1);
          log_printf(1,
                     "BNE rs1=%s=%" PRIx32 " rs2=%s=%" PRIx32 " imm=%" PRIx32
                     "\n",
                     rv_rname(i.b.rs1),
                     ctx->x[i.b.rs1],
                     rv_rname(i.b.rs2),
                     ctx->x[i.b.rs2],
                     imm);
          if (ctx->x[i.b.rs1] != ctx->x[i.b.rs2]) {
            ctx->pc_next = ctx->pc + rv_signext(imm, 12);
          }
          break;
        }
        case RV_BGE: {
          uint32_t imm = (i.b.imm_12 << 12) + (i.b.imm_11 << 11) +
                         (i.b.imm_10_5 << 5) + (i.b.imm_4_1 << 1);
          log_printf(1,
                     "BGE rs1=%s=%" PRIx32 " rs2=%s=%" PRIx32 " imm=%" PRIx32
                     "\n",
                     rv_rname(i.b.rs1),
                     ctx->x[i.b.rs1],
                     rv_rname(i.b.rs2),
                     ctx->x[i.b.rs2],
                     imm);
          if (ctx->x[i.b.rs1] >= ctx->x[i.b.rs2]) {
            ctx->pc_next = ctx->pc + imm;
          }
          break;
        }
        case RV_BGEU: {
          uint32_t imm = (i.b.imm_12 << 12) + (i.b.imm_11 << 11) +
                         (i.b.imm_10_5 << 5) + (i.b.imm_4_1 << 1);
          log_printf(1,
                     "BGEU rs1=%s=%" PRIx32 " rs2=%s=%" PRIx32 " imm=%" PRIx32
                     "\n",
                     rv_rname(i.b.rs1),
                     ctx->x[i.b.rs1],
                     rv_rname(i.b.rs2),
                     ctx->x[i.b.rs2],
                     imm);
          if (ctx->x[i.b.rs1] >= ctx->x[i.b.rs2]) {
            ctx->pc_next = ctx->pc + imm;
          }
          break;
        }
        default:
          die();
          return 1;
      }
      break;

    case RV_STORE:
      log_printf(1, "STORE ");
      switch (i.r.funct3) {
        case RV_SB: {
          uint32_t imm = i.s.imm_4_0 + (i.s.imm_11_5 << 5);
          log_printf(1,
                     "SB rd=%s funct3=%" PRIx8 " rs1=%s imm=%" PRIx32 "\n",
                     rv_rname(i.s.rs1),
                     i.s.funct3,
                     rv_rname(i.s.rs2),
                     imm);
          rv_write8(
            ctx, ctx->x[i.s.rs1] + rv_signext(imm, 11), ctx->x[i.s.rs2]);
          break;
        }
        case RV_SH: {
          uint32_t imm = i.s.imm_4_0 + (i.s.imm_11_5 << 5);
          log_printf(1,
                     "SH rd=%s funct3=%" PRIx8 " rs1=%s imm=%" PRIx32 "\n",
                     rv_rname(i.s.rs1),
                     i.s.funct3,
                     rv_rname(i.s.rs2),
                     imm);
          rv_write16(
            ctx, ctx->x[i.s.rs1] + rv_signext(imm, 11), ctx->x[i.s.rs2]);
          break;
        }
        case RV_SW: {
          uint32_t imm = i.s.imm_4_0 + (i.s.imm_11_5 << 5);
          log_printf(1,
                     "SW rd=%s funct3=%" PRIx8 " rs1=%s imm=%" PRIx32 "\n",
                     rv_rname(i.s.rs1),
                     i.s.funct3,
                     rv_rname(i.s.rs2),
                     imm);
          rv_write32(
            ctx, ctx->x[i.s.rs1] + rv_signext(imm, 11), ctx->x[i.s.rs2]);
          break;
        }
        default:
          die();
          return 1;
      }
      break;

    case RV_AUIPC:
      log_printf(1,
                 "AUIPC rd=%s imm31_12=%" PRIx32 "\n",
                 rv_rname(i.u.rd),
                 i.u.imm_31_12);
      if (i.u.rd)
        ctx->x[i.u.rd] = (i.u.imm_31_12 << 12) + ctx->pc;
      break;

    case RV_JAL: {
      uint32_t imm = (i.j.imm_20 << 20) + (i.j.imm_19_12 << 12) +
                     (i.j.imm11 << 11) + (i.j.imm_10_1 << 1);
      log_printf(1, "JAL rd=%s imm=%" PRIx32 "\n", rv_rname(i.j.rd), imm);
      if (i.j.rd)
        ctx->x[i.j.rd] = ctx->pc_next;
      ctx->pc_next = ctx->pc + rv_signext(imm, 20);
      //    log_printf(1, "JAL PC=%" PRIx32 "\n", ctx->pc_next);
      break;
    }

    case RV_JALR:
      log_printf(1,
                 "JALR rd=%s rs1=%s imm11_0=%" PRIx32 "\n",
                 rv_rname(i.i.rd),
                 rv_rname(i.i.rs1),
                 i.i.imm_11_0);
      if (i.i.rd)
        ctx->x[i.i.rd] = ctx->pc_next;
      ctx->pc_next = (rv_signext(i.i.imm_11_0, 11) + ctx->x[i.i.rs1]) & ~1;
      break;

    case RV_SYSTEM:
      switch (i.sy.funct3) {
        case RV_PRIV:
          switch (i.sy.funct12) {
            case RV_EBREAK:
              log_printf(1, "BREAK\n");
              if (ctx->ebreak) {
                ret = ctx->ebreak(ctx);
                break;
              }
              return 1;
              break;
            case RV_ECALL:
              log_printf(1,
                         "ECALL rd=%s rs1=%s a7=%" PRIx32 "\n",
                         rv_rname(i.sy.rd),
                         rv_rname(i.sy.rs1),
                         ctx->a7);
              if (ctx->ecall) {
                ret = ctx->ecall(ctx);
                break;
              }
              return 1;
              break;
            default:
              switch (i.r.funct7) {
                case 0x08:
                  switch (i.r.rs2) {
                    case 0x5:
                      log_printf(1, "WFI\n");
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
          }
          break;
        case RV_CSRRW:
          log_printf(1, "CSRRW\n");
          if (ctx->csr) {
            uint32_t val = ctx->x[i.csr.rs1_uimm];
            ret = ctx->csr(ctx, i.csr.csr, &val, i.csr.rd != 0, 1);
            if (i.csr.rd) {
              ctx->x[i.csr.rd] = val;
            }
            break;
          }
          return 1;
          break;
        case RV_CSRRS:
          log_printf(1, "CSRRS\n");
          if (ctx->csr) {
            uint32_t val = ctx->x[i.csr.rs1_uimm];
            ret = ctx->csr(ctx, i.csr.csr, &val, i.csr.rd != 0, 1);
            if (i.csr.rd) {
              ctx->x[i.csr.rd] = val;
            }
            break;
          }
          return 1;
          break;
        case RV_CSRRWI:
          log_printf(1, "CSRRWI\n");
          if (ctx->csr) {
            uint32_t val = i.csr.rs1_uimm;
            ret = ctx->csr(ctx, i.csr.csr, &val, i.csr.rd != 0, 1);
            if (i.csr.rd) {
              ctx->x[i.csr.rd] = val;
            }
            break;
          }
          return 1;
          break;
        default:
          die();
          return 1;
      }
      break;

#ifdef RV32A
    case RV_AMO:
      log_printf(1, "AMO ");
      switch (i.amo.funct5) {
        case RV_AMOADD: {
          log_printf(1,
                     "ADD rd=%s funct3=%" PRIx8 " rs1=%s rs2=%s\n",
                     rv_rname(i.amo.rd),
                     i.amo.funct3,
                     rv_rname(i.amo.rs1),
                     rv_rname(i.amo.rs2));
          uint32_t data = rv_read32(ctx, ctx->x[i.amo.rs1]);
          if (i.amo.rd)
            ctx->x[i.amo.rd] = data;
          data += ctx->x[i.s.rs2];
          rv_write32(ctx, ctx->x[i.s.rs1], data);
          break;
        }
        default:
          die();
          return 1;
      }
      break;
#endif

    case RV_MISC_MEM:
      log_printf(1, "MISC_MEM ");
      switch (i.r.funct3) {
        case RV_FENCE:
          log_printf(1,
                     "FENCE rd=%s funct3=%" PRIx8 " rs1=%s rs2=%s",
                     rv_rname(i.r.rd),
                     i.r.funct3,
                     rv_rname(i.r.rs1),
                     rv_rname(i.r.rs2));
          break;
        case RV_FENCEI:
          log_printf(1,
                     "FENCEI rd=%s funct3=%" PRIx8 " rs1=%s rs2=%s",
                     rv_rname(i.r.rd),
                     i.r.funct3,
                     rv_rname(i.r.rs1),
                     rv_rname(i.r.rs2));
          break;
        default:
          die();
          return 1;
      }
      break;

    default:
      printf("Unknown opc=%d\n", i.opc);
      die();
      return 1;
  }
  ctx->pc = ctx->pc_next;

  return ret;
}

#ifdef HAVE_GDBSTUB
static int
rsp_question(void* user)
{
  rv_ctx* ctx = (rv_ctx*)user;
  void* rsp = ctx->rsp;
  char* buf = "S05";
  rsp_send(rsp, buf, strlen(buf));
  return 0;
}

static int
rsp_stepi(void* user)
{
  rv_ctx* ctx = (rv_ctx*)user;
  int ret = 0;
  rv_step(ctx);
  ret = 1;
  return ret;
}

static int
rsp_cont(void* user)
{
  rv_ctx* ctx = (rv_ctx*)user;
  int ret = 0;
  while (1) {
    ret = rv_step(ctx);
    if (ret < 0)
      break;
  }
  rsp_question(ctx);
  return ret;
}

static int killed = 0;
int
rsp_kill(void* user)
{
  user = user;
  killed = 1;
  return 0;
}

static int intr = 0;
int
rsp_intr(void* user)
{
  rv_ctx* ctx = (rv_ctx*)user;
  void* rsp = ctx->rsp;
  intr = 1;
  rsp_stopped(rsp);
  return 0;
}

#define LEN32 (400 * 2)
static int
rsp_get_regs(void* user)
{
  rv_ctx* ctx = (rv_ctx*)user;
  void* rsp = ctx->rsp;
  char buf[LEN32 + 1];
  int len = LEN32;
  memset(buf, '0', len);
  for (int i = 1; i < RV_REGS; i++) {
    uint32_t be = htonl(ctx->x[i]);
    sprintf(buf + 8 * i, "%08" PRIx32, be);
  }
  rsp_send(rsp, buf, len);
  return 0;
}

static int
rsp_read_mem(void* user, size_t addr, size_t len)
{
  rv_ctx* ctx = (rv_ctx*)user;
  void* rsp = ctx->rsp;
  char* buf = malloc(len * 2);
  memset(buf, '0', len * 2);
  unsigned char* m = malloc(len);
  rv_read(ctx, m, addr, len);
  for (int i = 0; i < len; i++) {
    sprintf(buf + 2 * i, "%02x", m[i]);
  }
  free(m);
  rsp_send(rsp, buf, len * 2);
  free(buf);
  return 0;
}
#endif

rv_ctx*
rv_create(int api, rv_ctx_init init)
{
  if (api > RV_API)
    return 0;
  if (!init.read || !init.write) {
    return 0;
  }
  rv_ctx* ctx = calloc(1, sizeof(rv_ctx));
  ctx->init = init;
#ifdef HAVE_GDBSTUB
  if (ctx->init.rsp_port) {
    ctx->rsp = rsp_init(&(rsp_init_t) {
      .user = ctx, .port = ctx->init.rsp_port, .debug = ctx->init.rsp_debug,
      .question = rsp_question, .get_regs = rsp_get_regs,
      .read_mem = rsp_read_mem, .stepi = rsp_stepi, .cont = rsp_cont,
      .kill = rsp_kill, .intr = rsp_intr,
#if 0
      .intr = rsp_intr,
#endif
    });
  }
#endif
  return ctx;
}

int
rv_destroy(rv_ctx* ctx)
{
  if (!ctx)
    return 1;
#ifdef HAVE_GDBSTUB
  if (ctx->rsp) {
    rsp_cleanup(ctx->rsp);
  }
#endif
  free(ctx);
  return 0;
}

int
rv_execute(rv_ctx* ctx)
{
  int ret = 0;
  if (!ctx) {
    return 1;
  }
  if (ctx->rsp) {
    ret = rsp_execute(ctx->rsp);
  } else {
    while (1) {
      // rv_print_insn(ctx);
      ret = rv_step(ctx);
      if (ret) {
        break;
      }
    }
  }
  return ret;
}
