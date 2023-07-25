#define _GNU_SOURCE // for execvpe

#include "em_qcheck.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static int pipe_to_qemu[2];
static int pipe_from_qemu[2];
static int pipe_fromerr_qemu[2];

int rsp_sock = -1;
int
qinit(rv_ctx* ctx, char* esw)
{
  pipe(pipe_to_qemu);
  pipe(pipe_from_qemu);
  pipe(pipe_fromerr_qemu);

  int qpid = fork();
  switch (qpid) {
    case 0: {
      close(pipe_to_qemu[1]);
      close(pipe_from_qemu[0]);
      close(pipe_fromerr_qemu[0]);
      dup2(pipe_to_qemu[0], 0);
      dup2(pipe_from_qemu[1], 1);
      dup2(pipe_fromerr_qemu[1], 2);
      char* newargv[] = { "qemu-riscv32", "-g", "1234", "-R",
                          "0x2000000",    esw,  NULL };
      char* newenviron[] = { NULL };
      // we execvpe because we want to specify an empty environment
      execvpe(newargv[0], newargv, newenviron);
      perror("execvpe"); /* execvpe() returns only on error */
      exit(EXIT_FAILURE);
    }
    default:
      break;
  }
  close(pipe_to_qemu[0]);
  close(pipe_from_qemu[1]);
  close(pipe_fromerr_qemu[1]);

  sleep(1);     // FIXME: find better way to wait for spawned RSP server (qemu) to be ready
  struct sockaddr_in sa;
  rsp_sock = socket(PF_INET, SOCK_STREAM, 0);
  printf("{rsp_sock is %d}\n", rsp_sock);
  memset(&sa, 0, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_port = htons(1234);
  sa.sin_addr.s_addr = inet_addr("127.0.0.1");
  if (connect(rsp_sock, (struct sockaddr*)&sa, sizeof(sa))) {
    perror("connect");
    return 1;
  }
  printf("{connected}\n");
  return 0;
}

uint32_t
get32(char* p)
{
  uint32_t val = 0;
  // 01234567
  // deadbeef
  // efbeadde
  char buf[] = "0xXXXXXXXX";
  buf[2 + 0] = p[6 + 0];
  buf[2 + 1] = p[6 + 1];
  buf[2 + 2] = p[4 + 0];
  buf[2 + 3] = p[4 + 1];
  buf[2 + 4] = p[2 + 0];
  buf[2 + 5] = p[2 + 1];
  buf[2 + 6] = p[0 + 0];
  buf[2 + 7] = p[0 + 1];
  sscanf(buf, "%" SCNx32, &val);
  return val;
}

int
qcheck(rv_ctx* ctx)
{
  int ret = 0;
  write(rsp_sock, "+$g#67", 6);
  int reg = 1;
  rv_ctx ctx_qemu;
  while (1) {
    char buf[1024];
    size_t size = sizeof(buf);
    memset(buf, 0, size);
    size_t len = 0;
    while (len < size) {
      if (read(rsp_sock, buf + len, 1) <= 0) {
        return 1;
      }
      len++;
      if (buf[len - 1] == '#') {
        char c;
        read(rsp_sock, &c, 1);
        read(rsp_sock, &c, 1);
        break;
      }
    }
    char* str = buf;
    while (*str == '+') {
      str++;
    }
    if (*str++ != '$') {
      return 1;
    }
    while (1) {
      if (*str == '#') {
        break;
      }
      uint32_t val = get32(str);
      if (reg <= RV_REGS) {
        ctx_qemu.x[reg - 1] = val;
      }
      // printf("READ REG #%d = %" PRIx64 "\n", (int)reg, (uint64_t)val);
      reg++;
      str += 8;
      if (reg > RV_REGS) {
        break;
      }
  }
  break;
  }
  for (int reg = 1; reg < RV_REGS; reg++) {
    if (ctx_qemu.x[reg] != ctx->x[reg]) {
      printf("QEMU x%d=%08" PRIx32 " DIFFERENT from our x%d=%08" PRIx32 "!\n",
             reg,
             ctx_qemu.x[reg],
             reg,
             ctx->x[reg]);
      ret = 1;
      break;
    }
  }
  if (ret) {
    printf("%-8s%-8s\t%-12s   %-8s%-8s\n",
           "",
           "QEMU reg value",
           "",
           "",
           "Our reg value");
    for (int i = 1; i < RV_REGS; i++) {
      printf("%-8s0x%-8" PRIx32 "\t%-12" PRId32,
             rv_rname(i),
             ctx_qemu.x[i],
             ctx_qemu.x[i]);
      printf("%-3s", ctx_qemu.x[i] != ctx->x[i] ? "**" : "");
      printf(
        "%-8s0x%-8" PRIx32 "\t%-12" PRId32, rv_rname(i), ctx->x[i], ctx->x[i]);
      printf("\n");
    }
  }
  write(rsp_sock, "+$s#73", 6);
  char buf[1024];
  size_t len = 0, size = 1024;
  while (len < size) {
    if (read(rsp_sock, buf + len, 1) <= 0) {
      return 1;
    }
    len++;
    if (buf[len - 1] == '#') {
      char c;
      read(rsp_sock, &c, 1);
      read(rsp_sock, &c, 1);
      break;
    }
  }
  return ret;
}

int
qcleanup(rv_ctx* ctx)
{
  write(rsp_sock, "+$k#6b", 6);
  close(rsp_sock);
  return 0;
}
