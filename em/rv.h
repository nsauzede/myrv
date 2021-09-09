#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#define RV_REGS 32

// typedef  (*rv_cb_read)(uint32_t addr, );
typedef uint32_t (*rv_read32_cb)(uint32_t addr);

typedef struct rv_ctx {
  rv_read32_cb read32;
  uint32_t last_insn;
  uint32_t x[RV_REGS];
  // x[0] : zero register, always == 0, writes ignored
  // x[1] : link register (return address)
  // x[2] : stack pointer
  // x[5] : alternate link register
  uint32_t pc;
} rv_ctx;

int rv_init(rv_ctx *ctx);
int rv_execute(rv_ctx *ctx);
