#include <assert.h>
#include <stdio.h>

#include "rv.h"

uint32_t rv_read32(uint32_t addr) {
//   printf("blah\n");
  switch (addr) {
  case 0:
    return 0x00351793;
  default:
    return 0xffffffff;
  }
}

int rv_test() {
  // test bad input params
  assert(1 == rv_init(0));
  assert(1 == rv_execute(0));
  // test good input params
  rv_ctx ctx;
  assert(0 == rv_init(&ctx));
  assert(0 == ctx.pc); // test initial PC
  ctx.read32 = rv_read32;
  assert(0 == rv_execute(&ctx));
  assert(0x00351793 == ctx.last_insn);
  assert(4 == ctx.pc); // test incremented PC
  assert(0 == rv_execute(&ctx));
  assert(0xffffffff == ctx.last_insn);

  printf("RV Test OK\n");
  return 0;
}

int main() { return rv_test(); }
