#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#define RV_REGS 32

typedef uint32_t (*rv_read32_cb)(uint32_t addr);

typedef enum {
  // funct3
  RV_SLLI = 0x1,
  RV_SR_I = 0x5,
  RV_ADD_SUB = 0x0,
  RV_ADDI = 0x0,
  // funct7
  RV_S_LI = 0x00,
  RV_S_AI = 0x20,
  RV_ADD = 0x00,
  RV_SUB = 0x20,
  // opcode
  RV_LUI = 0x37,
  RV_AUIPC = 0x17,
  RV_JAL = 0x6f,
  RV_JALR = 0x67,
  RV_BRANCH = 0x63,
  RV_LOAD = 0x03,
  RV_STORE = 0x23,
  RV_OP_IMM = 0x13,
  RV_OP = 0x33,
  RV_MISC_MEM = 0x0f,
  RV_SYSTEM = 0x73,
} rv_opc;

typedef struct rv_ctx {
  rv_read32_cb read32;
  uint32_t last_insn;
  union {
    uint32_t x[RV_REGS];
    struct {
      uint32_t zero, ra, sp, gp, tp, t0, t1, t2, s0, s1, a0, a1, a2, a3, a4, a5,
          a6, a7, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, t3, t4, t5, t6;
    };
  };
  uint32_t pc;
} rv_ctx;

typedef union {
  uint32_t insn;
  uint32_t opc : 7;
  struct {
    uint32_t opc : 7, rd : 5, funct3 : 3, rs1 : 5, rs2 : 5, funct7 : 7;
  } r;
  struct {
    uint32_t opc : 7, rd : 5, funct3 : 3, rs1 : 5, imm_11_0 : 12;
  } i;
  struct {
    uint32_t opc : 7, imm_4_0 : 5, funct3 : 3, rs1 : 5, rs2 : 5, imm_11_5 : 7;
  } s;
  struct {
    uint32_t opc : 7, imm_11 : 1, imm_4_1 : 4, funct3 : 3, rs1 : 5, rs2 : 5,
        imm_10_5 : 6, imm_12 : 1;
  } b;
  struct {
    uint32_t opc : 7, rd : 5, imm_31_12 : 20;
  } u;
  struct {
    uint32_t opc : 7, rd : 5, imm_19_12 : 8, imm11 : 1, imm_10_1 : 10,
        imm_20 : 1;
  } j;
  struct {
    uint32_t opc : 7, rd : 5, funct3 : 3, rs1 : 5, imm_4_0 : 5, imm_11_5 : 7;
  } sh;
  struct {
    uint32_t opc : 7, rd : 5, imm_31_12 : 20;
  } laui;
} rv_insn;

int rv_init(rv_ctx *ctx);
int rv_execute(rv_ctx *ctx);
