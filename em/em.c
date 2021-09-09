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

  rv_insn i;
  i.insn = 0x00351793;
  printf("insn=%" PRIx32 "\n", i.insn);
  printf("func7=%" PRIx8 "\n", i.r.funct7);
  printf("rs2=%" PRIx8 "\n", i.r.rs2);
  printf("rs1=%" PRIx8 "\n", i.r.rs1);
  printf("funct3=%" PRIx8 "\n", i.r.funct3);
  printf("rd=%" PRIx8 "\n", i.r.rd);
  printf("opc=%" PRIx8 "\n", i.r.opc);

  return 0;
}
