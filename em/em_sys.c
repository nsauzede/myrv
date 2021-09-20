#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "em_sys.h"

#define die()                                                                  \
  do {                                                                         \
    printf("DIE! %s:%s:%d\n", __func__, __FILE__, __LINE__);                   \
    exit(1);                                                                   \
  } while (0)

static int g_log = 0;
#define log_printf(l, ...)                                                     \
  do {                                                                         \
    if (l <= g_log) {                                                          \
      printf(__VA_ARGS__);                                                     \
    }                                                                          \
  } while (0)

int em_ebreak(rv_ctx *ctx) {
  log_printf(1, "BREAK\n");
  return 1;
}

int em_ecall(rv_ctx *ctx) {
  switch (ctx->a7) {
  case 64: {
    log_printf(1, "write a1=%" PRIx32 " a2=%" PRIx32 " \\\n", ctx->a1, ctx->a2);
    char *buf = calloc(ctx->a2 + 1, 1);
    ctx->read(buf, ctx->a1, ctx->a2);
    ctx->a0 = printf("%s", buf);
    free(buf);
    break;
  }
  case 80: {
    typedef uint16_t rv_dev_t;
    typedef uint16_t rv_ino_t;
    typedef uint32_t rv_mode_t;
    typedef uint16_t rv_nlink_t;
    typedef uint16_t rv_uid_t;
    typedef uint16_t rv_gid_t;
    typedef uint32_t rv_off_t;
    typedef uint32_t rv_blksize_t;
    typedef uint32_t rv_blkcnt_t;
    typedef struct rv_timespec {
      uint64_t tv_sec, tv_nsec;
    } rv_timespec_t;
    typedef struct rv_stat {
      rv_dev_t st_dev;            /* ID of device containing file */
      rv_ino_t st_ino;            /* Inode number */
      rv_mode_t st_mode;          /* File type and mode */
      rv_nlink_t st_nlink;        /* Number of hard links */
      rv_uid_t st_uid;            /* User ID of owner */
      rv_gid_t st_gid;            /* Group ID of owner */
      rv_dev_t st_rdev;           /* Device ID (if special file) */
      rv_off_t st_size;           /* Total size, in bytes */
      rv_blksize_t st_blksize;    /* Block size for filesystem I/O */
      rv_blkcnt_t st_blocks;      /* Number of 512B blocks allocated */
      rv_timespec_t st_atim;      /* Time of last access */
      struct rv_timespec st_mtim; /* Time of last modification */
      struct rv_timespec st_ctim; /* Time of last status change */
#define st_atime st_atim.tv_sec   /* Backward compatibility */
#define st_mtime st_mtim.tv_sec
#define st_ctime st_ctim.tv_sec
    } rv_stat_t;
    rv_stat_t st;
    memset(&st, 0, sizeof(st));
    st.st_dev = 0x18;
    st.st_ino = 0x4;
    st.st_mode = 0x2190;
    st.st_nlink = 0x1;
    st.st_uid = 0x3e8;
    st.st_gid = 0x5;
    st.st_rdev = 0x8801;
    st.st_size = 0x0;
    st.st_blksize = 0x400;
    st.st_blocks = 0x0;
    ctx->write(&st, ctx->a1, sizeof(st));
    log_printf(1, "FIXME fstat a0=%" PRIx32 " a1=%" PRIx32 "\n", ctx->a0,
               ctx->a1);
    // return 1;
    break;
  }
  case 93: {
    log_printf(1, "exit a0=%" PRIx32 "\n", ctx->a0);
    return 1;
    break;
  }
  default:
    die();
    return 1;
  }
  return 0;
}
