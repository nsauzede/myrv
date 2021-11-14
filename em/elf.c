#include <elf.h>
#include <fcntl.h>
#include <inttypes.h>
#include <libelf.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void
do_elf32(Elf* e)
{
  Elf32_Ehdr* hdr = elf32_getehdr(e);
  printf("hdr32=%p\n", hdr);
  printf("entry=%08" PRIx32 "\n", hdr->e_entry);
  size_t nph;
  elf_getphdrnum(e, &nph);
  printf("%d program headers\n", (int)nph);
  Elf32_Phdr* ph = elf32_getphdr(e);
  for (size_t i = 0; i < nph; i++) {
    printf(" of=%" PRIx32, ph[i].p_offset);
    printf(" pa=%" PRIx32, ph[i].p_paddr);
    printf(" va=%" PRIx32, ph[i].p_vaddr);
    printf(" fs=%" PRIx32, ph[i].p_filesz);
    printf(" ms=%" PRIx32, ph[i].p_memsz);
    Elf_Data* d =
      elf_getdata_rawchunk(e, ph[i].p_offset, ph[i].p_filesz, ELF_T_PHDR);
    printf(" %08" PRIx64, *(uint64_t*)d);
    uint64_t* p = (uint64_t*)*(uint64_t*)d;
    printf(" %016" PRIx64, *p);
    printf("\n");
  }
}

int
main(int argc, char* argv[])
{
  char* fname = "esw";
  int arg = 1;
  if (arg < argc) {
    fname = argv[arg++];
  }
  elf_version(EV_CURRENT);
  int fd = open(fname, O_RDONLY, 0);
  Elf* e = elf_begin(fd, ELF_C_READ, 0);
  Elf_Kind ek = elf_kind(e);
  printf("elf kind=%s\n", ek == ELF_K_ELF ? "ELF" : "??");
  if (elf32_getehdr(e)) {
    do_elf32(e);
  }
  elf_end(e);
  close(fd);
  return 0;
}
