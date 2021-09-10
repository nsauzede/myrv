#include <stdint.h>
#include <stdio.h>

#include "rv.h"

int main() {
  rv_ctx ctx;
  rv_init(&ctx);
  while (1) {
    if (rv_execute(&ctx)) {
      printf("RV execution stopped\n");
      break;
    }
  }

  return 0;
}
