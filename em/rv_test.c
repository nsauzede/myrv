#include <stdio.h>
#include <assert.h>

#include "rv.h"

int rv_test() {
	// test bad input params
	assert(1 == rv_init(0));
	assert(1 == rv_execute(0));
	// test good input params
	rv_ctx ctx;
	assert(0 == rv_init(&ctx));
	assert(0 == ctx.pc);	// test initial PC
	assert(0 == rv_execute(&ctx));
	assert(1 == ctx.pc);	// test incremented PC
//	assert(0 == rv_execute(&ctx));
	printf("RV Test OK\n");
	return 0;
}

int main() {
	return rv_test();
}
