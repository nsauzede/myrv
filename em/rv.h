#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#define RV_REGS 32

// typedef  (*rv_cb_read)(uint32_t addr, );
typedef uint32_t (*rv_read32_cb)(uint32_t addr);

typedef enum {
  // funct3
  RV_SL_ = 0x1,
  RV_SR_ = 0x5,
  // funct7
  RV_S_L = 0x00,
  RV_S_A = 0x20,
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
  uint32_t x[RV_REGS];
  // x[0] zero : hardwired zero, always == 0, writes ignored
  // x[1] ra : link register (return address)
  // x[2] sp : stack pointer

  // x[5] t0 : alternate link register
  // x[10-11] a0-1 : function arg / ret val
  // x[12-17] a2-7 : function arg
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
} rv_insn;

int rv_init(rv_ctx *ctx);
int rv_execute(rv_ctx *ctx);
