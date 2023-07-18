#ifndef RV_H__
#define RV_H__

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

// We add to the end of general regs to match with gdb output
// and ease of coding
#define RV_REGS (32 + 1)

#define RV32M
#define RV32A

typedef enum
{
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
#ifdef RV32A
  RV_AMO = 0x2f,
#endif

  // funct3
  // OP
  RV_ADD_SUB = 0x0,
  RV_SLL = 0x1,
  RV_SLTU = 0x3,
  RV_XOR = 0x4,
  RV_SRL = 0x5,
  RV_OR = 0x6,
  RV_AND = 0x7,
  // OP-IMM
  RV_ADDI = 0x0,
  RV_SLTI = 0x2,
  RV_SLTIU = 0x3,
  RV_XORI = 0x4,
  RV_ORI = 0x6,
  RV_ANDI = 0x7,
  // SHIFT
  RV_SLLI = 0x1,
  RV_SR_I = 0x5,
  // BRANCH
  RV_BEQ = 0x0,
  RV_BNE = 0x1,
  RV_BLT = 0x4,
  RV_BGE = 0x5,
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
  RV_PRIV = 0x0, // ECALL, EBREAK, MRET
  RV_CSRRW = 0x1,
  RV_CSRRS = 0x2,
  RV_CSRRWI = 0x5,
  // MISC-MEM
  RV_FENCE = 0x0,
  RV_FENCEI = 0x1,

#ifdef RV32M
  RV_ADD_SUB_MUL = 0x0,
  RV_SLL_MULH = 0x1,
  RV_SLTU_MULHU = 0x3,
  RV_XOR_DIV = 0x4,
  RV_SRL_DIVU = 0x5,
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
  RV_MULH = 0x01,
  RV_MULHU = 0x01,
  RV_DIV = 0x01,
  RV_DIVU = 0x01,
  RV_REM = 0x01,
#endif
  RV_WFI = 0x08,
// funct5
#ifdef RV32A
  RV_AMOADD = 0x0,
#endif
  // funct12
  RV_ECALL = 0x000,
  RV_EBREAK = 0x001,
  RV_MRET = 0x302,
} rv_opc;

typedef union
{
  uint32_t insn;
  uint32_t opc : 7;
  struct
  {
    uint32_t opc : 7, rd : 5, funct3 : 3, rs1 : 5, rs2 : 5, funct7 : 7;
  } r;
  struct
  {
    uint32_t opc : 7, rd : 5, funct3 : 3, rs1 : 5, imm_11_0 : 12;
  } i;
  struct
  {
    uint32_t opc : 7, imm_4_0 : 5, funct3 : 3, rs1 : 5, rs2 : 5, imm_11_5 : 7;
  } s;
  struct
  {
    uint32_t opc : 7, imm_11 : 1, imm_4_1 : 4, funct3 : 3, rs1 : 5, rs2 : 5,
      imm_10_5 : 6, imm_12 : 1;
  } b;
  struct
  {
    uint32_t opc : 7, rd : 5, imm_31_12 : 20;
  } u;
  struct
  {
    uint32_t opc : 7, rd : 5, imm_19_12 : 8, imm11 : 1, imm_10_1 : 10,
      imm_20 : 1;
  } j;
  struct
  {
    uint32_t opc : 7, rd : 5, funct3 : 3, rs1 : 5, imm_4_0 : 5, imm_11_5 : 7;
  } sh;
  struct
  {
    uint32_t opc : 7, rd : 5, funct3 : 3, rs1 : 5, funct12 : 12;
  } sy;
  struct
  {
    uint32_t opc : 7, rd : 5, funct3 : 3, rs1 : 5, rs2 : 5, rl : 1, aq : 1,
      funct5 : 5;
  } amo;
  struct
  {
    uint32_t opc : 7, rd : 5, funct3 : 3, rs1_uimm : 5, csr : 12;
  } csr;
} rv_insn;

struct rv_ctx;

typedef uint32_t (*rv_read_cb)(void* dest, uint32_t addr, uint32_t size);
typedef uint32_t (*rv_write_cb)(const void* src, uint32_t addr, uint32_t size);
typedef int (*rv_ebreak_cb)(struct rv_ctx* ctx);
typedef int (*rv_ecall_cb)(struct rv_ctx* ctx);
typedef int (*rv_mret_cb)(struct rv_ctx* ctx);
typedef int (*rv_csr_cb)(struct rv_ctx* ctx,
                         uint16_t csr,
                         uint32_t* inout,
                         int read,
                         int write);

#define RV_API 0

typedef struct rv_ctx_init
{
  rv_read_cb read;
  rv_write_cb write;
  rv_ebreak_cb ebreak;
  rv_ecall_cb ecall;
  rv_mret_cb mret;
  rv_csr_cb csr;
#ifdef HAVE_GDBSTUB
  int rsp_port;
  int rsp_debug;
#endif
} rv_ctx_init;

typedef struct rv_ctx
{
  union
  {
    rv_ctx_init init;
    struct
    {
      rv_read_cb read;
      rv_write_cb write;
      rv_ebreak_cb ebreak;
      rv_ecall_cb ecall;
      rv_mret_cb mret;
      rv_csr_cb csr;
    };
  };

  uint32_t last_insn;
  union
  {
    uint32_t x[RV_REGS];
    struct
    {
      uint32_t zero, ra, sp, gp, tp, t0, t1, t2, s0, s1, a0, a1, a2, a3, a4, a5,
        a6, a7, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, t3, t4, t5, t6, pc;
    };
  };
  uint32_t pc_next;
#ifdef HAVE_GDBSTUB
  void* rsp;
#endif
} rv_ctx;

int
rv_set_log(rv_ctx* ctx, int log);
rv_ctx*
rv_create(int api, rv_ctx_init init);
int
rv_destroy(rv_ctx* ctx);
int
rv_step(rv_ctx* ctx);
int
rv_execute(rv_ctx* ctx);
void
rv_print_regs(rv_ctx* ctx);
char*
rv_rname(uint8_t reg);

#endif /*RV_H__*/
