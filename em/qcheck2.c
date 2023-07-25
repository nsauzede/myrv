#define _GNU_SOURCE // for execvpe

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef void rv_ctx;

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

static int pipe_to_qemu[2];
static int pipe_from_qemu[2];
static int pipe_fromerr_qemu[2];
int
qinit(rv_ctx* ctx)
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
      // qemu-riscv32 -g 1234 -s 0x200 esw&
      char* newargv[] = { "qemu-riscv32", "-g", "1234", "esw", NULL };
      char* newenviron[] = { NULL };

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
  printf("{qpid is %d}\n", qpid);
  sleep(1); // FIXME: find better way to wait for spawned RSP server (qemu) to
            // be ready
  int rsp_sock;
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
  /*
  +$?#3f
  +$g#67
  +$s#73
  +$k#6b
  */
#if 0
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
#endif
  write(rsp_sock, "+$g#67", 6);
  int reg = 1;
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
      // printf("len=%zd\n", len);
      if (buf[len - 1] == '#') {
        char c;
        read(rsp_sock, &c, 1);
        read(rsp_sock, &c, 1);
        break;
      }
    }
    // printf("[%s]", buf);
    char* ptr = buf;
    while (*ptr == '+') {
      ptr++;
    }
    if (*ptr++ != '$') {
      return 1;
    }
    while (1) {
      if (*ptr == '#') {
        break;
      }
      uint32_t val = get32(ptr);
      if (reg == 32) {
        printf("{reg pc=%" PRIx32 "}\n", val);

      } else {
        printf("{reg#%d=%" PRIx32 "}\n", reg, val);
      }
      reg++;
      ptr += 8;
    }
    break;
  }
  write(rsp_sock, "+$k#6b", 6);
  close(rsp_sock);
  printf("{Done}\n");
  return 0;
}

int
qcheck(rv_ctx* ctx)
{
  return 1;
}

int
main()
{
  printf("qinit returned %d\n", qinit(0));
  qcheck(0);
  return 0;
}
