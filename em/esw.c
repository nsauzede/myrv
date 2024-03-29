#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __riscv
#define EBREAK asm volatile("ebreak")

void
first_func()
{
  EBREAK;
}

void
start();
// void _start() { start(); }

int
foo(int a, int b);
void
start()
{
  int* a = (int*)0x2000;
  int* b = (int*)0x2004;
  int* c = (int*)0x2008;

  *c = foo(*a, *b);
  // *(uint32_t *)0x2008 = foo(*a, *b);

  EBREAK;
  exit(1);
}
#endif
int
foo(int a, int b)
{
#ifdef __riscv
#define GREETING "Hello riscv\n"
#else
#define GREETING "Hello world\n"
#endif
  write(1, GREETING, 12);
  struct stat st;
  memset(&st, 0, sizeof(st));
#ifdef __riscv
  // return sizeof(st.st_dev); // 2
  // return sizeof(st.st_ino);//2
  // return sizeof(st.st_mode);//4
  // return sizeof(st.st_nlink); // 2
  // return sizeof(st.st_uid); // 2
  // return sizeof(st.st_gid); // 2
  // return sizeof(st.st_rdev); // 2
  // return sizeof(st.st_size); // 4
  // return sizeof(st.st_blksize); // 4
  // return sizeof(st.st_blocks); // 4
  // return sizeof(st.st_atim); // 16
  // return sizeof(st.st_mtim); // 16
  // return sizeof(st.st_ctim); // 16
  // return sizeof(st.st_atim.tv_sec); // 8
  // fstat(0, &st);
  // return st.st_dev; // 24
  // return st.st_ino; // 6
  // return st.st_mode; // 144
  // return st.st_nlink; // 1
  // return st.st_uid; // 232
  // return st.st_gid; // 5
  // return st.st_rdev; // 3
  // return st.st_size; // 0
  // return st.st_blksize; // 0
  // return st.st_blocks; // 0
  // return st.st_atim.tv_sec; // 106
  // return st.st_atim.tv_nsec; // 0
  // return st.st_mtim.tv_sec; // 227
  // return st.st_mtim.tv_nsec; // 0
  // return st.st_ctim.tv_sec; // 0
  // return st.st_ctim.tv_nsec; // 0
#else
  // return sizeof(st.st_dev); // 8
  // return sizeof(st.st_ino);//8
  // return sizeof(st.st_mode);//4
  // return sizeof(st.st_nlink); // 8
  // return sizeof(st.st_atim); // 16
  // return sizeof(st.st_atim.tv_sec); // 8
  fstat(0, &st);
#endif

  // return sizeof(st.st_atim);//16
  // return sizeof(st);//144
#ifdef __riscv
#if 0
  printf("dev=%" PRIx32 "\n", (uint32_t)st.st_dev);
  printf("ino=%" PRIx32 "\n", (uint32_t)st.st_ino);
  printf("mode=%" PRIx32 "\n", (uint32_t)st.st_mode);
  printf("nlink=%" PRIx32 "\n", (uint32_t)st.st_nlink);
  printf("uid=%" PRIx32 "\n", (uint32_t)st.st_uid);
  printf("gid=%" PRIx32 "\n", (uint32_t)st.st_gid);
  printf("rdev=%" PRIx32 "\n", (uint32_t)st.st_rdev);
  printf("size=%" PRIx32 "\n", (uint32_t)st.st_size);
  printf("blksize=%" PRIx32 "\n", (uint32_t)st.st_blksize);
  printf("blocks=%" PRIx32 "\n", (uint32_t)st.st_blocks);
  printf("atim.sec=%" PRIx32 "\n", (uint32_t)st.st_atim.tv_sec);
  printf("atim.nsec=%" PRIx32 "\n", (uint32_t)st.st_atim.tv_nsec);
#endif
#else
  printf("dev=%" PRIx32 "\n", (uint32_t)st.st_dev);
  printf("ino=%" PRIx32 "\n", (uint32_t)st.st_ino);
  printf("mode=%" PRIx32 "\n", (uint32_t)st.st_mode);
  printf("nlink=%" PRIx32 "\n", (uint32_t)st.st_nlink);
  printf("uid=%" PRIx32 "\n", (uint32_t)st.st_uid);
  printf("gid=%" PRIx32 "\n", (uint32_t)st.st_gid);
  printf("rdev=%" PRIx32 "\n", (uint32_t)st.st_rdev);
  printf("size=%" PRIx32 "\n", (uint32_t)st.st_size);
  printf("blksize=%" PRIx32 "\n", (uint32_t)st.st_blksize);
  printf("blocks=%" PRIx32 "\n", (uint32_t)st.st_blocks);
  printf("atim.sec=%" PRIx32 "\n", (uint32_t)st.st_atim.tv_sec);
  printf("atim.nsec=%" PRIx32 "\n", (uint32_t)st.st_atim.tv_nsec);
#endif
  //  puts("hello puts\n");
  // printf("hello %s\n", "printf");
  return a + b;
  // return -5;
}
int
main()
{
  return foo(-2, -3);
}
