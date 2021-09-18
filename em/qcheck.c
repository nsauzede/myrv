#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

typedef void rv_ctx;

int get_line(int fd, char *buf, size_t size) {
  char *ptr = buf;
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

int wait_for(int fd, char *prompt, char *prefix, int quit_on_output,
             int print_all) {
  while (1) {
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    if (get_line(fd, buf, sizeof(buf)))
      return 1;
    if (strstr(buf, prompt))
      break;
    if (buf[0] == '&') {
      if (quit_on_output) {
        printf("%s", buf);
        return 1;
      }
    }
    if (print_all)
      printf("%s", buf);
    if (prefix && !strncmp(prefix, buf, strlen(prefix)))
      printf("%s", buf);
  }
  return 0;
}

static int pipe_to_qemu[2];
static int pipe_from_qemu[2];
static int pipe_fromerr_qemu[2];
int qinit(rv_ctx *ctx) {
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
    // qemu-riscv32 -g 1234 -s 0x200 esw&
    char *newargv[] = {
        "/usr/bin/qemu-riscv32", "-g", "1234", "-s", "1234", "esw", NULL};
    char *newenviron[] = {NULL};

    execve(newargv[0], newargv, newenviron);
    perror("execve"); /* execve() returns only on error */
    exit(EXIT_FAILURE);
  }
  default:
    break;
  }
  close(pipe_to_qemu[0]);
  close(pipe_from_qemu[1]);
  close(pipe_fromerr_qemu[1]);
  printf("{qpid is %d}\n", qpid);

  int pipe_to_gdb[2];
  int pipe_from_gdb[2];
  int pipe_fromerr_gdb[2];
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
    // riscv32-unknown-elf-gdb -q -nx -ex target\ remote\ 127.0.0.1:1234 esw
    // -i=mi
    char *newargv[] = {"/home/nico/perso/git/riscv-gnu-toolchain/the_install/"
                       "bin/riscv32-unknown-elf-gdb",
                       "-q",
                       "-nx",
                       "-ex",
                       "target remote 127.0.0.1:1234",
                       "esw",
                       "-i=mi",
                       NULL};
    char *newenviron[] = {NULL};

    execve(newargv[0], newargv, newenviron);
    perror("execve"); /* execve() returns only on error */
    exit(EXIT_FAILURE);
  }
  default:
    break;
  }
  close(pipe_to_gdb[0]);
  close(pipe_from_gdb[1]);
  close(pipe_fromerr_gdb[1]);
  printf("{gpid is %d}\n", gpid);

  if (wait_for(pipe_from_gdb[0], "(gdb)", 0, 1, 0))
    return 1;
  write(pipe_to_gdb[1], "disp/i $pc\n", 11);
  if (wait_for(pipe_from_gdb[0], "(gdb)", 0, 0, 0))
    return 1;
  write(pipe_to_gdb[1], "info r\n", 7);
  int reg = 1;
  while (1) {
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    if (get_line(pipe_from_gdb[0], buf, sizeof(buf)))
      break;
    // printf("%s", buf);
    char *str = buf;
    while (1) {
      char *next = strstr(str, "~\"");
      if (!next)
        break;
      uint32_t val = 0;
      //~"ra             0
      sscanf(next + 2, "%" SCNx32, &val);
      if (reg == 32) {
        printf("{reg pc=%" PRIx32 "}\n", val);

      } else {
        printf("{reg#%d=%" PRIx32 "}\n", reg, val);
      }
      reg++;
      str = strchr(next, '\t');
      if (!str)
        break;
    }
    if (strstr(buf, "(gdb)"))
      break;
  }
  write(pipe_to_gdb[1], "disp\n", 5);
  if (wait_for(pipe_from_gdb[0], "(gdb)", "~\"=> ", 0, 0))
    return 1;
  write(pipe_to_gdb[1], "kill\n", 5);
  write(pipe_to_gdb[1], "quit\n", 5);
  printf("{Done}\n");
  return 0;
}

int qcheck(rv_ctx *ctx) { return 1; }

int main() {
  qinit(0);
  qcheck(0);
}
