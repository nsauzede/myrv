#include <stdio.h>

#include "rv.h"

int main() {
	rv_ctx ctx;
	while (1) {
		if (rv_execute(&ctx)) {
			printf("RV execution stopped\n");
			break;
		}
	}
	return 0;
}
