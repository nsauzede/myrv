#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#ifdef __riscv
#define EBREAK asm volatile("ebreak")

void first_func() { EBREAK; }

void start();
// void _start() { start(); }

int foo(int a, int b);
void start() {
  int *a = (int *)0x2000;
  int *b = (int *)0x2004;
  int *c = (int *)0x2008;

  *c = foo(*a, *b);
  // *(uint32_t *)0x2008 = foo(*a, *b);

  EBREAK;
  exit(1);
}
#endif
int foo(int a, int b) {
#ifdef __riscv
#define GREETING "Hello riscv\n"
#else
#define GREETING "Hello world\n"
#endif
  // write(1, GREETING, 12);
  // printf("hello printf\n");
  return a + b;
  // return -5;
}
int main() { return foo(-2, -3); }
