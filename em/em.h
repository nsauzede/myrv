#ifndef EM_H_
#define EM_H_

#include <stdint.h>

#define EM_VERSION "0.1"

uint32_t rv_read(void *dest, uint32_t addr, uint32_t size);
uint32_t rv_write(const void *src, uint32_t addr, uint32_t size);

#endif/*EM_H_*/
