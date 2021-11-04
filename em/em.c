#define _GNU_SOURCE // for execvpe

#include "em.h"

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "em_sys.h"

#ifdef HAVE_ELF
#include "em_elf.h"
#endif

#ifdef HAVE_GDBSTUB
#include "em_gdbstub.h"
#endif

#ifdef HAVE_QCHECK
#include "em_qcheck.h"
#endif

#include "rv.h"

uint32_t mem_start = 0;
uint32_t mem_len = 0x2000000;
unsigned char *mem = 0;

static uint32_t rom[] = {
  0x00000297,
  0x02828613,
  0xf1402573,
  0x0202a583,
  0x0182a283,
  0x00028067,
  0x80000000, //start_addr,
  0x00000000,
  0x87e00000,		//fdt_addr,
  0x00000000,
};
#define ROM_START 0x1000
#define ROM_LEN sizeof(rom)

uint32_t rv_read(void *dest, uint32_t addr, uint32_t size) {
  if (!dest) {
    return 1;
  }
  uint32_t ret = 0;
  if ((addr >= ROM_START) && (addr + size <= ROM_START + ROM_LEN)) {
    memcpy(dest, (void *)rom + addr - ROM_START, size);
  } else if ((addr >= mem_start) && (addr + size <= mem_start + mem_len)) {
    memcpy(dest, mem + addr - mem_start, size);
  } else {
  	printf("Illegal read addr=0x%" PRIx32 " size=%" PRIu32 "\n", addr, size);
  	exit(1);
  }
  return ret;
}

uint8_t rv_read8(uint32_t addr) {
  uint8_t val = 0;
  rv_read(&val, addr, sizeof(val));
  return val;
}

uint32_t rv_read32(uint32_t addr) {
  uint32_t val = 0;
  rv_read(&val, addr, sizeof(val));
  return val;
}

//uint32_t uart0 = 0;
uint32_t uart0 = 0x300000;	// same UART as in franzflasch/riscv_em

uint32_t rv_write(const void *src, uint32_t addr, uint32_t size) {
  if (!!uart0 && (addr == uart0) && (size == 4)) {
    printf("%c", *(unsigned char *)src);
    fflush(stdout);
  } else if (src && (addr >= mem_start) && (addr + size <= mem_start + mem_len)) {
    memcpy(mem + addr - mem_start, src, size);
  } else {
  	printf("Illegal write addr=0x%" PRIx32 " size=%" PRIu32 "\n", addr, size);
  	exit(1);
  }
  return size;
}

uint32_t rv_write32(uint32_t addr, uint32_t val) {
  return rv_write(&val, addr, sizeof(val));
}

void help(int argc, char *argv[]) {
  printf("myrv/em - Simple RISC-V emulator - version %s\n", EM_VERSION);
  printf("Copyright (c) 2021 Nicolas Sauzede\n");
  printf("\n");
  printf("Usage: %s [options] <esw_file <start_pc <start_sp> > >\n", argv[0]);
  printf("\n");
#ifdef HAVE_ELF
  printf("'esw_file' can be ELF or Flat Binary\n");
#else
  printf("'esw_file' must be a Flat Binary\n");
#endif
  printf("\n");
  printf("Options:\n");
  printf("  --help\tDisplay this information.\n");
  printf("  -v\tSet verbosity level 1.\n");
  printf("  -vv\tSet verbosity level 2.\n");
  printf("  -m\tSet custom memory length (in bytes).\n");
  printf("  -d\tLoad dtb at 0x87e00000 length 1414 (experimental).\n");
#ifdef HAVE_GDBSTUB
  printf("  -s\tAccept gdb connection on tcp::1235. Freeze CPU at startup.\n");
#endif
#ifdef HAVE_QCHECK
  printf("  -q\tCompare execution with qemu+gdb.\n");
#endif
}

int main(int argc, char *argv[]) {
  uint32_t start_pc = mem_start;
  uint32_t start_sp = mem_start + mem_len;
  int override_pc = 0;
  int override_sp = 0;
#ifdef HAVE_ELF
  char *esw_fin = "esw";
#else
  char *esw_fin = "em_esw.bin";
#endif
  char *dtb_fin = 0;
#ifdef HAVE_GDBSTUB
  int do_gdbstub = 0;
  int gport = 1235;
  void *gctx = 0;
#endif
#ifdef HAVE_QCHECK
  int do_qcheck = 0;
#endif
  int log = 0;
  int pos = 0;
  int arg = 1;
  while (arg < argc) {
    if (!strcmp(argv[arg], "--help")) {
      arg++;
      help(argc, argv);
      exit(0);
    }
    if (!strcmp(argv[arg], "-v")) {
      arg++;
      log++;
      continue;
    }
    if (!strcmp(argv[arg], "-vv")) {
      arg++;
      log += 2;
      continue;
    }
#ifdef HAVE_GDBSTUB
    if (!strcmp(argv[arg], "-s")) {
      arg++;
      do_gdbstub = 1;
      continue;
    }
#endif
#ifdef HAVE_QCHECK
    if (!strcmp(argv[arg], "-q")) {
      arg++;
      do_qcheck = 1;
      continue;
    }
#endif
    if (!strcmp(argv[arg], "-m")) {
      arg++;
      sscanf(argv[arg++], "%" SCNx32, &mem_len);
      continue;
    }
    if (!strcmp(argv[arg], "-d")) {
      arg++;
      dtb_fin = argv[arg++];
      continue;
    }
    if (pos == 0) {
      esw_fin = argv[arg++];
      pos++;
      continue;
    }
    if (pos == 1) {
      sscanf(argv[arg++], "%" SCNx32, &start_pc);
      override_pc = 1;
      pos++;
      continue;
    }
    if (pos == 2) {
      sscanf(argv[arg++], "%" SCNx32, &start_sp);
      override_sp = 1;
      pos++;
      continue;
    }
  }
  if (!override_sp) {
    start_sp = mem_start + mem_len;
  }

  mem = calloc(mem_len, 1);
#ifdef HAVE_ELF
  uint32_t pc = start_pc;
  if (!elf_load(esw_fin, &pc)) {
    printf("[Loaded ELF %s]\n", esw_fin);
    if (!override_pc) {
      start_pc = pc;
    }
  } else {
#endif
    FILE *in = fopen(esw_fin, "rb");
    if (!in) {
      printf("Failed to open %s\n", esw_fin);
      exit(1);
    }
    fread(mem, mem_len, 1, in);
    fclose(in);
    printf("[Loaded flat binary %s]\n", esw_fin);
#ifdef HAVE_ELF
  }
#endif
  if (dtb_fin) {
    FILE *in = fopen(dtb_fin, "rb");
    if (!in) {
      printf("Failed to open %s\n", dtb_fin);
      exit(1);
    }
    fread(mem + 0x87e00000 - mem_start, 1414, 1, in);
    fclose(in);
    printf("[Loaded DTB %s]\n", dtb_fin);
  }

  rv_ctx_init init = {.read = rv_read,
                      .write = rv_write,
                      .ebreak = em_ebreak,
                      .ecall = em_ecall,
                      .csr = em_csr};
  rv_ctx *ctx = rv_create(RV_API, init);
  rv_set_log(ctx, log);

  ctx->sp = start_sp;
  ctx->pc = start_pc;

  // TODO: Fill the stack with user asrgs/env
  // until we get same final SP and stack contents as in qemu
  if (!override_sp) {
    ctx->sp = 0x1ffff40;
    rv_write32(ctx->sp, 1);
  }

#ifdef HAVE_GDBSTUB
  if (do_gdbstub) {
    gctx = ginit(ctx, gport);
    if (!gctx) {
      printf("[ginit failed]\n");
      return 1;
    }
  }
#endif

#ifdef HAVE_QCHECK
  if (do_qcheck) {
    if (qinit(ctx, esw_fin)) {
      printf("[qinit failed]\n");
      return 1;
    }
  }
#endif

  if (!strcmp("em_esw", esw_fin) || !strcmp("em_esw.bin", esw_fin)) {
    printf("[Setting input params at 0x2000: -2 and -3 (will be added as a "
           "result))]\n");
    rv_write32(0x2000, -2);
    rv_write32(0x2004, -3);
  }
  while (1) {
#ifdef HAVE_GDBSTUB
    if (do_gdbstub) {
      gstatus_t gs = gstatus(gctx);
      switch (gs) {
      case GS_QUIT:
        printf("[gstatus quit]\n");
        return 1;
      case GS_EXECUTE:
        break;
      default:
        printf("[gstatus unknown %d]\n", gs);
        return 1;
      }
      if (!gctx) {
      }
    }
#endif
#ifdef HAVE_QCHECK
    if (do_qcheck) {
      static int count = 0;
      if (qcheck(ctx)) {
        printf("[qcheck #%d failed]\n", count);
        return 1;
      }
      count++;
    }
#endif
    if (rv_execute(ctx)) {
      break;
    }
  }
#ifdef HAVE_GDBSTUB
  if (do_gdbstub) {
    gcleanup(gctx);
  }
#endif
#ifdef HAVE_QCHECK
  if (do_qcheck) {
    printf("[qcheck PASSED]\n");
    qcleanup(ctx);
  }
#endif
  if (!strcmp("em_esw", esw_fin) || !strcmp("em_esw.bin", esw_fin)) {
    int val = rv_read32(0x2008);
    printf("[Memory state at finish : %d (should be -5)]\n", val);
  } else if (!strcmp("esw", esw_fin)) {
    printf("[A0 reg at finish : %" PRId32 " (should be -5)]\n", ctx->a0);
  }
  rv_destroy(ctx);
  return 0;
}
