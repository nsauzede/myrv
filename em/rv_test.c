#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "rv.h"

static uint32_t mem2000 = 0;
static uint32_t mem2004 = 0;
static uint32_t mem2008 = 0;

static int
rv_write32(uint32_t addr, uint32_t val)
{
  switch (addr) {
    case 0x2000:
      mem2000 = val;
      break;
    case 0x2004:
      mem2004 = val;
      break;
    case 0x2008:
      mem2008 = val;
      break;
    default:
      return 1;
  }
  return 0;
}

static uint32_t
rv_read32(uint32_t addr)
{
  switch (addr) {
    case 0 * 4:
      return 0x00351793; // slli a5,a0,0x3
    case 1 * 4:
      return 0x40a78533; // sub a0,a5,a0
    case 2 * 4:
      return 0x00000013; // nop (addi x0,x0,0)
    case 3 * 4:
      return 0x00002537; // lui	a0,0x2
    case 4 * 4:
      return 0x00050513; // mv	a0,a0
    case 5 * 4:
      return 0x00052283; // lw	t0,0(a0) # 2000 <DATA_BASE>
    case 6 * 4:
      return 0x00452303; // lw	t1,4(a0)
    case 7 * 4:
      return 0x006283b3; // add t2,t0,t1
    case 8 * 4:
      return 0x00752423; // sw	t2,8(a0)

    case 0x2000: {
      return mem2000;
    }
    case 0x2004: {
      return mem2004;
    }
    case 0x2008: {
      return mem2008;
    }
    default:
      return 0x00100073; // break
  }
}

static uint32_t
rv_write(const void* src, uint32_t addr, uint32_t size)
{
  switch (size) {
    case 4:
      return rv_write32(addr, *(uint32_t*)src);
    default:
      printf("%s: NOTIMP size %d\n", __func__, size);
      exit(1);
  }
}

static uint32_t
rv_read(void* dest, uint32_t addr, uint32_t size)
{
  switch (size) {
    case 4:
      *(uint32_t*)dest = rv_read32(addr);
      return 4;
    default:
      printf("%s: NOTIMP size %d\n", __func__, size);
      exit(1);
  }
}

static int
rv_test()
{
  // test bad input params
  assert(0 == rv_create(RV_API + 1, (rv_ctx_init){ 0 }));
  assert(0 == rv_create(RV_API, (rv_ctx_init){ 0 }));
  assert(1 == rv_step(0));
  // test good input params
  rv_ctx* ctx;
  assert(0 != (ctx = rv_create(RV_API, (rv_ctx_init){ rv_read, rv_write })));
  assert(0 == ctx->pc); // test initial PC
  assert(0 == ctx->a0); // test initial a0
  assert(0 == ctx->a5); // test initial a5
  assert(0 == ctx->t0); // test initial t0
  assert(0 == ctx->t1); // test initial t1
  assert(0 == ctx->t2); // test initial t2
  ctx->a0 = 1;
  rv_write32(0x2000, -2);
  rv_write32(0x2004, -3);
  assert(1 == ctx->a0);    // test input a0
  assert(1 == ctx->x[10]); // test input a0
  assert(0 == rv_step(ctx));
  assert(0x00351793 == ctx->last_insn);
  assert(4 == ctx->pc); // test incremented PC
  assert(8 == ctx->a5); // test shift+move
  assert(0 == rv_step(ctx));
  assert(0x40a78533 == ctx->last_insn);
  assert(8 == ctx->pc); // test incremented PC
  assert(7 == ctx->a0); // test sub+move
  assert(0 == rv_step(ctx));
  assert(0x00000013 == ctx->last_insn);

  assert(0 == rv_step(ctx));
  assert(0x00002537 == ctx->last_insn);

  assert(0 == rv_step(ctx));
  assert(0x00050513 == ctx->last_insn);

  assert(0 == rv_step(ctx));
  assert(0x00052283 == ctx->last_insn);
  assert(-2 == ctx->t0); // test lw

  assert(0 == rv_step(ctx));
  assert(0x00452303 == ctx->last_insn);
  assert(-3 == ctx->t1); // test lw

  assert(0 == rv_step(ctx));
  assert(0x006283b3 == ctx->last_insn);
  assert(-5 == ctx->t2); // test add+move

  assert(0 == rv_step(ctx));
  assert(0x00752423 == ctx->last_insn);
  assert(-5 == rv_read32(0x2008)); // test sw

  assert(1 == rv_step(ctx));
  assert(0x00100073 == ctx->last_insn);

  assert(0 == rv_destroy(ctx));

  printf("RV Test OK\n");
  return 0;
}

int
main()
{
  return rv_test();
}
