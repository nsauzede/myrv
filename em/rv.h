#include <inttypes.h>

#define RV_REGS 32
typedef struct rv_ctx {
	uint32_t x[RV_REGS];
	// x[0] : zero register, always == 0
	// x[1] : link register (return address)
	// x[2] : stack pointer
	// x[5] : alternate link register
	uint32_t pc;
} rv_ctx;

int rv_init(rv_ctx *ctx);
int rv_execute(rv_ctx *ctx);
