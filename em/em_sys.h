
#include "rv.h"

#define CSR_MTVEC 0x305
#define CSR_MSCRATCH 0x340
#define CSR_MHARTID 0xf14

int
em_ebreak(rv_ctx* ctx);
int
em_ecall(rv_ctx* ctx);
int
em_csr(rv_ctx* ctx, uint16_t csr, uint32_t* inout, int read, int write);
