#include <unistd.h>

#ifdef __riscv
#define EBREAK asm volatile("ebreak")

void first_func() { EBREAK; }

void start();
void _start() { start(); }

int foo(int a, int b);
void start() {
  int *a = (int *)0x2000;
  int *b = (int *)0x2004;
  int *c = (int *)0x2008;

  *c = foo(*a, *b);

  EBREAK;
}
#endif
int foo(int a, int b) {
#ifdef __riscv
  write(1, "Hello riscv\n", 12);
#else
  write(1, "Hello world\n", 12);
#endif
  return a + b;
}
int main() { return foo(-2, -3); }
