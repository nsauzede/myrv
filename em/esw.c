
void start() {
	int *a = (int *)0x2000;
	int *b = (int *)0x2004;
	int *c = (int *)0x2008;
	*c = *a + *b;
	asm volatile(
		"ebreak"
	);
}
