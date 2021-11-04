#ifndef EM_ELF_H_
#define EM_ELF_H_

#include <stdint.h>

int elf_load(char *fname, uint32_t *entry);

#endif/*EM_ELF_H_*/
