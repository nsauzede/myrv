#include <unistd.h>

#ifdef __riscv
int foo(int a, int b);
void start() {
  int *a = (int *)0x2000;
  int *b = (int *)0x2004;
  int *c = (int *)0x2008;

  *c = foo(*a, *b);

  asm volatile("ebreak");
}

#endif
int foo(int a, int b) {
  write(1, "Hello world\n", 12);
  return a + b;
}
int main() { return foo(-2, -3); }
