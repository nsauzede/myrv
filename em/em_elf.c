#include "em_elf.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <libelf.h>

#include "em.h"

int elf_load(char *fname, uint32_t *entry) {
  elf_version(EV_CURRENT);
  int fd = open(fname, O_RDONLY, 0);
  Elf *e = elf_begin(fd, ELF_C_READ, 0);
  Elf_Kind ek = elf_kind(e);
  if (ek != ELF_K_ELF) {
    return 1;
  }
  if (!elf32_getehdr(e)) {
    return 1;
  }
  Elf32_Ehdr *hdr = elf32_getehdr(e);
  if (entry) {
    *entry = hdr->e_entry;
  }
  size_t nph;
  elf_getphdrnum(e, &nph);
  Elf32_Phdr *ph = elf32_getphdr(e);
  for (size_t i = 0; i < nph; i++) {
    Elf_Data *d =
        elf_getdata_rawchunk(e, ph[i].p_offset, ph[i].p_filesz, ELF_T_PHDR);
    uint32_t *p = *(uint32_t **)d;
    char *zero = calloc(1, ph[i].p_memsz);
    rv_write(zero, ph[i].p_vaddr, ph[i].p_memsz);
    free(zero);
    rv_write(p, ph[i].p_vaddr, ph[i].p_filesz);
  }

  elf_end(e);
  close(fd);
  return 0;
}
