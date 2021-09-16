#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "rv.h"

uint32_t mem_start = 0;
uint32_t mem_len = 0x4000;
unsigned char *mem = 0;

uint32_t rv_read(void *dest, uint32_t addr, uint32_t size) {
  uint32_t ret = 0;
  if (dest && (addr >= mem_start) && (addr + size <= mem_start + mem_len)) {
    memcpy(dest, mem + addr - mem_start, size);
  }
  return ret;
}

uint32_t rv_read32(uint32_t addr) {
  uint32_t val = 0;
  if ((addr >= mem_start) && (addr + 4 <= mem_start + mem_len)) {
    memcpy(&val, mem + addr - mem_start, 4);
  }
  return val;
}

int rv_write32(uint32_t addr, uint32_t val) {
  if ((addr >= mem_start) && (addr + 4 <= mem_start + mem_len)) {
    memcpy(mem + addr - mem_start, &val, 4);
    return 0;
  }
  return 1;
}

int main(int argc, char *argv[]) {
  uint32_t start_pc = 0;
  uint32_t start_sp = 0x2000;
  char *fin = "em_esw.bin";
  int arg = 1;
  if (arg < argc) {
    fin = argv[arg++];
    if (arg < argc) {
      sscanf(argv[arg++], "%" SCNx32, &start_pc);
      if (arg < argc) {
        sscanf(argv[arg++], "%" SCNx32, &start_sp);
      }
    }
  }
  mem = calloc(mem_len, 1);
  FILE *in = fopen(fin, "rb");
  fread(mem, mem_len, 1, in);
  fclose(in);

  rv_ctx ctx;
  rv_init(&ctx, rv_read, rv_read32, rv_write32);
  ctx.sp = start_sp;
  ctx.pc = start_pc;

  rv_write32(0x2000, -2);
  rv_write32(0x2004, -3);
  while (1) {
    if (rv_execute(&ctx)) {
      printf("RV execution stopped\n");
      break;
    }
  }
  int val = rv_read32(0x2008);
  printf("%d\n", val);

  return 0;
}
