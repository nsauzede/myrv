#include <assert.h>
#include <stdio.h>

#include "rv.h"

uint32_t rv_read32(uint32_t addr) {
  switch (addr) {
  case 0:
    return 0x00351793; // slli a5,a0,0x3
  case 4:
    return 0x40a78533; // sub a0,a5,a0
  case 8:
    return 0x00008067; // jalr ?? (ret)
  default:
    return 0x00000073; // break
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
  assert(0 == ctx.a0); // test initial a0
  assert(0 == ctx.a5); // test initial a5
  ctx.a0 = 1;
  assert(1 == ctx.a0); // test input a0
  assert(1 == ctx.x[10]); // test input a0
  ctx.read32 = rv_read32;
  assert(0 == rv_execute(&ctx));
  assert(0x00351793 == ctx.last_insn);
  assert(4 == ctx.pc); // test incremented PC
  assert(8 == ctx.a5); // test shift+move
  assert(0 == rv_execute(&ctx));
  assert(0x40a78533 == ctx.last_insn);
  assert(0 == rv_execute(&ctx));
  assert(0x00008067 == ctx.last_insn);
  assert(0 == rv_execute(&ctx));
  assert(0x00000073 == ctx.last_insn);

  printf("RV Test OK\n");
  return 0;
}

int main() { return rv_test(); }
