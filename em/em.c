#include <fcntl.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
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
uint32_t mem_len = 0x4000;
unsigned char *mem = 0;

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
    uint64_t *p = (uint64_t *)*(uint64_t *)d;
    // printf(" %016" PRIx64, *p);
    // printf("\n");
    rv_write(p, ph[i].p_vaddr, ph[i].p_filesz);
  }

  elf_end(e);
  close(fd);
  return 0;
}
#endif

int main(int argc, char *argv[]) {
  uint32_t start_pc = 0;
  uint32_t start_sp = 0x2000;
  char *fin = "esw";
  int log = 0;
  int pos = 0;
  int arg = 1;
  while (arg < argc) {
    if (!strcmp(argv[arg], "-v")) {
      arg++;
      log++;
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
  rv_init(&ctx, rv_read, rv_read32, rv_write32);
  ctx.sp = start_sp;
  ctx.pc = start_pc;

  rv_write32(0x2000, -2);
  rv_write32(0x2004, -3);
  while (1) {
    if (rv_execute(&ctx)) {
      // printf("RV execution stopped\n");
      break;
    }
  }
  int val = rv_read32(0x2008);
  printf("[Memory state at finish : %d (should be -5)]\n", val);

  return 0;
}
