#include <fcntl.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define HAVE_ELF
#ifdef HAVE_ELF
#include <libelf.h>
#endif

#include "rv.h"

uint32_t mem_start = 0;
// uint32_t mem_len = 0x4000;
uint32_t mem_len = 0x2000000;
unsigned char *mem = 0;

int do_qcheck = 0;

int get_line(int fd, char *buf, size_t size) {
  char *ptr = buf;
  size_t len = 0;
  while (1) {
    if (len >= size)
      break;
    if (read(fd, ptr, 1) <= 0) {
      return 1;
    }
    if (*ptr == '\n')
      break;
    ptr++;
    len++;
  }

  return 0;
}

int wait_for(int fd, char *prompt, char *prefix, int quit_on_output,
             int print_all) {
  // print_all = 1;
  while (1) {
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    if (get_line(fd, buf, sizeof(buf)))
      return 1;
    if (strstr(buf, prompt))
      break;
    if (buf[0] == '&') {
      //   printf("%s", buf);
      if (quit_on_output)
        return 1;
    }
    if (print_all)
      printf("%s", buf);
    if (prefix && !strncmp(prefix, buf, strlen(prefix)))
      printf("%s", buf);
  }
  return 0;
}

static int pipe_to_gdb[2];
static int pipe_from_gdb[2];
static int pipe_fromerr_gdb[2];
static int pipe_to_qemu[2];
static int pipe_from_qemu[2];
static int pipe_fromerr_qemu[2];

int qinit(rv_ctx *ctx) {
  pipe(pipe_to_qemu);
  pipe(pipe_from_qemu);
  pipe(pipe_fromerr_qemu);

  int qpid = fork();
  switch (qpid) {
  case 0: {
    close(pipe_to_qemu[1]);
    close(pipe_from_qemu[0]);
    close(pipe_fromerr_qemu[0]);
    dup2(pipe_to_qemu[0], 0);
    dup2(pipe_from_qemu[1], 1);
    dup2(pipe_fromerr_qemu[1], 2);
    // qemu-riscv32 -g 1234 -s 0x200 esw&
    char *newargv[] = {"/usr/bin/qemu-riscv32", "-g", "1234",
                       //  "-s",
                       //  "0x1000",
                       "-R", "0x2000000", "esw", NULL};
    char *newenviron[] = {NULL};

    execve(newargv[0], newargv, newenviron);
    perror("execve"); /* execve() returns only on error */
    exit(EXIT_FAILURE);
  }
  default:
    break;
  }
  close(pipe_to_qemu[0]);
  close(pipe_from_qemu[1]);
  close(pipe_fromerr_qemu[1]);
  printf("{qpid is %d}\n", qpid);

  pipe(pipe_to_gdb);
  pipe(pipe_from_gdb);
  pipe(pipe_fromerr_gdb);

  int gpid = fork();
  switch (gpid) {
  case 0: {
    close(pipe_to_qemu[1]);
    close(pipe_from_qemu[0]);
    close(pipe_fromerr_qemu[0]);
    close(pipe_to_gdb[1]);
    close(pipe_from_gdb[0]);
    close(pipe_fromerr_gdb[0]);
    dup2(pipe_to_gdb[0], 0);
    dup2(pipe_from_gdb[1], 1);
    dup2(pipe_fromerr_gdb[1], 2);
    // riscv32-unknown-elf-gdb -q -nx -ex target\ remote\ 127.0.0.1:1234 esw
    // -i=mi
    char *newargv[] = {"/home/nico/perso/git/riscv-gnu-toolchain/the_install/"
                       "bin/riscv32-unknown-elf-gdb",
                       "-q",
                       "-nx",
                       "-ex",
                       "target remote 127.0.0.1:1234",
                       "esw",
                       "-i=mi",
                       NULL};
    char *newenviron[] = {NULL};

    execve(newargv[0], newargv, newenviron);
    perror("execve"); /* execve() returns only on error */
    exit(EXIT_FAILURE);
  }
  default:
    break;
  }
  close(pipe_to_gdb[0]);
  close(pipe_from_gdb[1]);
  close(pipe_fromerr_gdb[1]);
  printf("{gpid is %d}\n", gpid);

  if (wait_for(pipe_from_gdb[0], "(gdb)", 0, 1, 0))
    return 1;
  // write(pipe_to_gdb[1], "disp/i $pc\n", 11);
  // if (wait_for(pipe_from_gdb[0], "(gdb)", 0, 0, 0))
  //   return 1;
  printf("{Done}\n");
  return 0;
}

int qcheck(rv_ctx *ctx) {
  int ret = 0;
  static int count = 0;
  printf("[qcheck #%d]\n", count++);
  write(pipe_to_gdb[1], "info r\n", 7);
  int reg = 1;
  rv_ctx ctx_qemu;
  while (1) {
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    if (get_line(pipe_from_gdb[0], buf, sizeof(buf)))
      break;
    // printf("buf=[%s]\n", buf);
    char *str = buf;
    while (1) {
      char *next = strstr(str, "~\"");
      if (!next)
        break;
      // printf("next=[%s]\n", next);
      uint32_t val = 0;
      //~"ra             0
      sscanf(next + 17, "%" SCNx32, &val);
      if (reg == 32) {
        ctx_qemu.pc = val;
        // if (val != ctx->pc) {
        //   printf("QEMU PC DIFFERENT!\n");
        //   ret = 1;
        //   break;
        // }
      } else if (reg < 32) {
        // printf("{reg#%d=%" PRIx32 "}\n", reg, val);
        ctx_qemu.x[reg] = val;
        // if (reg != 2 && val != ctx->x[reg]) {
        //   printf("QEMU x%d=%08" PRIx32 " DIFFERENT from our x%d=%08" PRIx32
        //          "!\n",
        //          reg, val, reg, ctx->x[reg]);
        //   ret = 1;
        //   break;
        // }
      }
      if (ret)
        break;
      reg++;
      str = strchr(next, '\t');
      if (!str)
        break;
    }
    if (ret)
      break;
    if (strstr(buf, "(gdb)"))
      break;
  }
  for (int reg = 1; reg < RV_REGS; reg++) {
    if (
        // reg != 2 &&
        ctx_qemu.x[reg] != ctx->x[reg]) {
      printf("QEMU x%d=%08" PRIx32 " DIFFERENT from our x%d=%08" PRIx32 "!\n",
             reg, ctx_qemu.x[reg], reg, ctx->x[reg]);
      ret = 1;
      break;
    }
  }
  if (!ret) {
    if (ctx_qemu.pc != ctx->pc) {
      printf("QEMU PC=%08" PRIx32 " DIFFERENT from our PC=%08" PRIx32 "!\n",
             ctx_qemu.pc, ctx->pc);
      ret = 1;
    }
  }
  if (ret) {
    // 0x1ffff40        3
    printf("%-15s%-8s\t%-8s\t   \t%-15s\n", "", "QEMU reg value", "",
           "Our reg value");
    for (int i = 1; i < RV_REGS; i++) {
      printf("%-15s0x%-8" PRIx32 "\t%-8" PRId32, rv_rname(i), ctx_qemu.x[i],
             ctx_qemu.x[i]);
      printf("\t%3s\t", ctx_qemu.x[i] != ctx->x[i] ? "***" : "");
      printf("0x%-8" PRIx32 "\t%-8" PRId32, ctx->x[i], ctx->x[i]);
      printf("\n");
    }
    printf("%-15s0x%-8" PRIx32 "\n", "pc", ctx->pc);
  } else {
    // printf("Our regs\n");
    rv_print_regs(ctx);
  }
  // write(pipe_to_gdb[1], "disp\n", 5);
  // if (wait_for(pipe_from_gdb[0], "(gdb)", "~\"=> ", 0, 0))
  //   return 1;
  write(pipe_to_gdb[1], "si\n", 3);
  if (wait_for(pipe_from_gdb[0], "(gdb)", 0, 0, 0))
    return 1;
  if (wait_for(pipe_from_gdb[0], "*stopped,reason=\"end-stepping-range\"", 0, 0,
               1))
    return 1;
  if (wait_for(pipe_from_gdb[0], "(gdb)", 0, 0, 0))
    return 1;
  return ret;
}

int qcleanup(rv_ctx *ctx) {
  write(pipe_to_gdb[1], "kill\n", 5);
  if (wait_for(pipe_from_gdb[0], "(gdb)", 0, 0, 0))
    return 1;
  write(pipe_to_gdb[1], "quit\n", 5);
  if (wait_for(pipe_from_gdb[0], "(gdb)", 0, 0, 0))
    return 1;
  return 0;
}

uint32_t rv_read(void *dest, uint32_t addr, uint32_t size) {
  uint32_t ret = 0;
  if (dest && (addr >= mem_start) && (addr + size <= mem_start + mem_len)) {
    memcpy(dest, mem + addr - mem_start, size);
  }
  return ret;
}

uint32_t rv_write(const void *src, uint32_t addr, uint32_t size) {
  uint32_t ret = 0;
  if (src && (addr >= mem_start) && (addr + size <= mem_start + mem_len)) {
    memcpy(mem + addr - mem_start, src, size);
  }
  return ret;
}

uint8_t rv_read8(uint32_t addr) {
  uint8_t val = 0;
  if ((addr >= mem_start) && (addr + 1 <= mem_start + mem_len)) {
    memcpy(&val, mem + addr - mem_start, 1);
  }
  return val;
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

#ifdef HAVE_ELF
int elf_load(char *fname, uint32_t *entry) {
  elf_version(EV_CURRENT);
  int fd = open(fname, O_RDONLY, 0);
  Elf *e = elf_begin(fd, ELF_C_READ, 0);
  Elf_Kind ek = elf_kind(e);
  if (ek != ELF_K_ELF) {
    return 1;
  }
  // printf("elf kind=%s\n", ek == ELF_K_ELF ? "ELF" : "??");
  if (!elf32_getehdr(e)) {
    return 1;
  }
  Elf32_Ehdr *hdr = elf32_getehdr(e);
  // printf("hdr32=%p\n", hdr);
  // printf("entry=%08" PRIx32 "\n", hdr->e_entry);
  if (entry) {
    *entry = hdr->e_entry;
  }
  size_t nph;
  elf_getphdrnum(e, &nph);
  // printf("%d program headers\n", (int)nph);
  Elf32_Phdr *ph = elf32_getphdr(e);
  for (size_t i = 0; i < nph; i++) {
    // printf(" of=%" PRIx32, ph[i].p_offset);
    // printf(" pa=%" PRIx32, ph[i].p_paddr);
    // printf(" va=%" PRIx32, ph[i].p_vaddr);
    // printf(" fs=%" PRIx32, ph[i].p_filesz);
    // printf(" ms=%" PRIx32, ph[i].p_memsz);
    Elf_Data *d =
        elf_getdata_rawchunk(e, ph[i].p_offset, ph[i].p_filesz, ELF_T_PHDR);
    // printf(" %08" PRIx64, *(uint64_t *)d);
    uint32_t *p = *(uint32_t **)d;
    printf(" Program#%zd: %08" PRIx32, i, *p);
    printf("\n");
    char *zero = calloc(1, ph[i].p_memsz);
    rv_write(zero, ph[i].p_vaddr, ph[i].p_memsz);
    free(zero);
    rv_write(p, ph[i].p_vaddr, ph[i].p_filesz);
  }

  elf_end(e);
  close(fd);
  return 0;
}
#endif

int main(int argc, char *argv[]) {
  uint32_t start_pc = mem_start;
  uint32_t start_sp = mem_len;
  char *fin = "em_esw";
  int log = 0;
  int pos = 0;
  int arg = 1;
  while (arg < argc) {
    if (!strcmp(argv[arg], "-q")) {
      arg++;
      do_qcheck = 1;
      continue;
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
    if (pos == 0) {
      fin = argv[arg++];
      pos++;
      continue;
    }
    if (pos == 1) {
      sscanf(argv[arg++], "%" SCNx32, &start_pc);
      pos++;
      continue;
    }
    if (pos == 2) {
      sscanf(argv[arg++], "%" SCNx32, &start_sp);
      pos++;
      continue;
    }
  }

  mem = calloc(mem_len, 1);
#ifdef HAVE_ELF
  if (!elf_load(fin, &start_pc)) {
    printf("[Loaded ELF]\n");
  } else {
#endif
    FILE *in = fopen(fin, "rb");
    fread(mem, mem_len, 1, in);
    fclose(in);
    printf("[Loaded flat binary]\n");
#ifdef HAVE_ELF
  }
#endif

  rv_ctx ctx;
  rv_set_log(&ctx, log);
  rv_init(&ctx, rv_read, rv_write, rv_read8, rv_read32, rv_write32);
  ctx.sp = start_sp;
  ctx.pc = start_pc;

  // TODO: Fill the stack with user asrgs/env
  // until we get same final SP and stack contents as in qemu
  ctx.sp = 0x1ffff40;
  rv_write32(ctx.sp, 1);

  if (do_qcheck) {
    if (qinit(&ctx)) {
      printf("[qinit failed]\n");
      return 1;
    }
  }

  if (!do_qcheck) {
    printf("[Setting input params at 0x2000: -2 and -3 (will be added as a "
           "result))]\n");
    rv_write32(0x2000, -2);
    rv_write32(0x2004, -3);
  }
  while (1) {
    if (do_qcheck) {
      static int count = 0;
      if (qcheck(&ctx)) {
        printf("[qcheck #%d failed]\n", count);
        return 1;
      }
      count++;
    }
    if (rv_execute(&ctx)) {
      // printf("RV execution stopped\n");
      break;
    }
  }
  int val = rv_read32(0x2008);
  printf("[Memory state at finish : %d (should be -5)]\n", val);

  return 0;
}
