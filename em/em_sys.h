
#include "rv.h"

#define CSR_SATP 0x180
#define CSR_MSTATUS 0x300
#define CSR_MEDELEG 0x302
#define CSR_MIDELEG 0x303
#define CSR_MIE 0x304
#define CSR_MTVEC 0x305
#define CSR_MSCRATCH 0x340
#define CSR_MEPC 0x341
#define CSR_PMPCFG0 0x3a0
#define CSR_PMPADDR0 0x3b0
#define CSR_MHARTID 0xf14

int
em_ebreak(rv_ctx* ctx);
int
em_ecall(rv_ctx* ctx);
int
em_mret(rv_ctx* ctx);
int
em_csr(rv_ctx* ctx, uint16_t csr, uint32_t* inout, int read, int write);
