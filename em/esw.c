int foo(int a, int b);
void start() {
  int *a = (int *)0x2000;
  int *b = (int *)0x2004;
  int *c = (int *)0x2008;

  *c = foo(*a, *b);

  asm volatile("ebreak");
}

int foo(int a, int b) { return a + b; }
int main() {
	return 0;
}
