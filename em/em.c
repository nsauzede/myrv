#include <stdint.h>
#include <stdio.h>

#include "rv.h"

uint32_t rv_read32(uint32_t addr) {
	return 0;
}

int rv_write32(uint32_t addr, uint32_t val) {
	return 0;
}

int main() {
  rv_ctx ctx;
  rv_init(&ctx, rv_read32, rv_write32);
  while (1) {
    if (rv_execute(&ctx)) {
      printf("RV execution stopped\n");
      break;
    }
  }

  return 0;
}
