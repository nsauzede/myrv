#define _GNU_SOURCE // for execvpe

#include "em_qcheck.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int
get_line(int fd, char* buf, size_t size)
{
  char* ptr = buf;
  size_t len = 0;
  while (1) {
    if (len >= size)
      break;
    if (read(fd, ptr, 1) <= 0) {
      return 1;
    }
    if (*ptr == '\n')
      break;
    ptr++;
    len++;
  }

  return 0;
}

static int
wait_for(int fd, char* prompt, char* prefix, int quit_on_output, int print_all)
{
  while (1) {
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    if (get_line(fd, buf, sizeof(buf)))
      return 1;
    if (strstr(buf, prompt))
      break;
    if (buf[0] == '&') {
      if (quit_on_output)
        return 1;
    }
    if (print_all)
      printf("(all) %s", buf);
    if (prefix && !strncmp(prefix, buf, strlen(prefix)))
      printf("(prefix) %s", buf);
  }
  return 0;
}

static int pipe_to_gdb[2];
static int pipe_from_gdb[2];
static int pipe_fromerr_gdb[2];
static int pipe_to_qemu[2];
static int pipe_from_qemu[2];
static int pipe_fromerr_qemu[2];

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

  pipe(pipe_to_gdb);
  pipe(pipe_from_gdb);
  pipe(pipe_fromerr_gdb);

  int gpid = fork();
  switch (gpid) {
    case 0: {
      close(pipe_to_qemu[1]);
      close(pipe_from_qemu[0]);
      close(pipe_fromerr_qemu[0]);
      close(pipe_to_gdb[1]);
      close(pipe_from_gdb[0]);
      close(pipe_fromerr_gdb[0]);
      dup2(pipe_to_gdb[0], 0);
      dup2(pipe_from_gdb[1], 1);
      dup2(pipe_fromerr_gdb[1], 2);
      char* newargv[] = {
        "riscv32-unknown-elf-gdb",      "-q",  "-nx",   "-ex",
        "target remote 127.0.0.1:1234", "esw", "-i=mi", NULL
      };

      execvp(newargv[0], newargv);
      perror("execvp"); /* execvp() returns only on error */
      exit(EXIT_FAILURE);
    }
    default:
      break;
  }
  close(pipe_to_gdb[0]);
  close(pipe_from_gdb[1]);
  close(pipe_fromerr_gdb[1]);

  if (wait_for(pipe_from_gdb[0], "(gdb)", 0, 1, 0))
    return 1;
  return 0;
}

int
qcheck(rv_ctx* ctx)
{
  int ret = 0;
  write(pipe_to_gdb[1], "info r\n", 7);
  int reg = 1;
  rv_ctx ctx_qemu;
  while (1) {
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    if (get_line(pipe_from_gdb[0], buf, sizeof(buf)))
      break;
    char* str = buf;
    while (1) {
      char* next = strstr(str, "~\"");
      if (!next)
        break;
      uint32_t val = 0;
      sscanf(next + 17, "%" SCNx32, &val);
      if (reg <= RV_REGS) {
        ctx_qemu.x[reg] = val;
      }
      // printf("READ REG #%d = %" PRIx64 "\n", (int)reg, (uint64_t)val);
      reg++;
      str = strchr(next, '\t');
      if (!str)
        break;
    }
    if (strstr(buf, "(gdb)"))
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
  write(pipe_to_gdb[1], "si\n", 3);
  if (wait_for(pipe_from_gdb[0], "(gdb)", 0, 0, 0))
    return 1;
  if (wait_for(pipe_from_gdb[0], "*stopped,reason=\"", 0, 0, 0))
    return 1;
  if (wait_for(pipe_from_gdb[0], "(gdb)", 0, 0, 0))
    return 1;
  return ret;
}

int
qcleanup(rv_ctx* ctx)
{
  write(pipe_to_gdb[1], "kill\n", 5);
  if (wait_for(pipe_from_gdb[0], "(gdb)", 0, 0, 0))
    return 1;
  write(pipe_to_gdb[1], "quit\n", 5);
  if (wait_for(pipe_from_gdb[0], "(gdb)", 0, 0, 0))
    return 1;
  return 0;
}
