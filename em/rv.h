#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#define RV_REGS 32

#define RV32M

typedef enum {
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
  // funct3
  // OP
  RV_ADD_SUB = 0x0,
  RV_SLL = 0x1,
  RV_SLTU = 0x3,
  RV_XOR = 0x4,
  RV_OR = 0x6,
  RV_AND = 0x7,
  // OP-IMM
  RV_ADDI = 0x0,
  RV_ORI = 0x6,
  RV_ANDI = 0x7,
  // SHIFT
  RV_SLLI = 0x1,
  RV_SR_I = 0x5,
  // BRANCH
  RV_BEQ = 0x0,
  RV_BNE = 0x1,
  RV_BLT = 0x4,
  RV_BLTU = 0x6,
  RV_BGEU = 0x7,
  // LOAD
  RV_LH = 0x1,
  RV_LW = 0x2,
  RV_LBU = 0x4,
  RV_LHU = 0x5,
  // STORE
  RV_SB = 0x0,
  RV_SH = 0x1,
  RV_SW = 0x2,
  // SYSTEM
  RV_PRIV = 0x0,

#ifdef RV32M
  RV_ADD_SUB_MUL = 0x0,
  RV_SLL_MULH = 0x1,
  RV_SLTU_MULHU = 0x3,
  RV_XOR_DIV = 0x4,
  RV_OR_REM = 0x6,
  RV_AND_REMU = 0x7,
#endif
  // funct7
  RV_S_LI = 0x00,
  RV_S_AI = 0x20,
  RV_ADD = 0x00,
  RV_SUB = 0x20,
#ifdef RV32M
  RV_MUL = 0x01,
  RV_MULHU = 0x01,
  RV_DIV = 0x01,
  RV_REM = 0x01,
#endif
  // funct12
  RV_ECALL = 0x000,
  RV_EBREAK = 0x001,
} rv_opc;

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
    uint32_t opc : 7, rd : 5, funct3 : 3, rs1 : 5, funct12 : 12;
  } sy;
} rv_insn;

typedef uint32_t (*rv_read_cb)(void *dest, uint32_t addr, uint32_t size);
typedef uint32_t (*rv_write_cb)(const void *src, uint32_t addr, uint32_t size);
typedef uint8_t (*rv_read8_cb)(uint32_t addr);
typedef uint32_t (*rv_read32_cb)(uint32_t addr);
typedef int (*rv_write32_cb)(uint32_t addr, uint32_t val);

typedef struct rv_ctx {
  rv_read_cb read;
  rv_write_cb write;
  rv_read8_cb read8;
  rv_read32_cb read32;
  rv_write32_cb write32;
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

int rv_set_log(rv_ctx *ctx, int log);
int rv_init(rv_ctx *ctx, rv_read_cb rv_read, rv_write_cb rv_write,
            rv_read8_cb rv_read8, rv_read32_cb rv_read32,
            rv_write32_cb rv_write32);
int rv_execute(rv_ctx *ctx);
void rv_print_regs(rv_ctx *ctx);
char *rv_rname(uint8_t reg);
