#include <stdio.h>
#include <string.h>

#include "cdfs.h"
#include "gddrive.h"
#include "notlibc.h"

START_EXTERN_C
#include "lwip/api.h"
END_EXTERN_C

#ifndef __SDCARD__

#define MAXFD 64

static struct {
  int serial;
  int result;
  int cmd;
  union {
    char filename[64];
    struct {
      int fd;
      int pos;
      int len;
    } rd;
  } data;
} cmd;

static int current_serial=4711;

static int readpos[MAXFD+1];
static char replybuf[2048];
static int replylen;
static struct netconn *cmd_conn = NULL;

static void virtcdhdlr(struct netbuf *buf)
{
  u16_t size = netbuf_len(buf);
  if(size >= 8) {
    struct { int serial, result; } res;
    netbuf_copy_partial(buf, &res, 8, 0);
    if(res.serial == cmd.serial) {
      cmd.result = res.result;
      if(size > 8)
	netbuf_copy_partial(buf, replybuf, size-8, 8);
      replylen = size-8;
    }
  }
}

static int docmd(int command, const void *data, int sz)
{
  struct netbuf *buf;
  if(!cmd_conn) return -1;
  cmd.serial = ++current_serial;
  cmd.result = -1;
  cmd.cmd = command;
  if(sz)
    memcpy(&cmd.data, data, sz);
  buf = netbuf_new();
  if(buf == NULL) return -1;
  netbuf_ref(buf, &cmd, 12+sz);
  for(;;) {
    void *r;
    netconn_send(cmd_conn, buf);
    if(sys_arch_mbox_fetch(cmd_conn->recvmbox, &r, 200) && r)
      do {
	virtcdhdlr(r);
	netbuf_delete(r);
      } while(cmd.result != -1 &&
	      sys_arch_mbox_fetch(cmd_conn->recvmbox, &r, 1) && r);
    if(cmd.result != -1) {
      netbuf_delete(buf);
      return cmd.result;
    }
  }
}

EXTERN_C int open(const char *path, int mode, ...)
{
  int res;
#ifdef FS_DEBUG
  printf("open(%s,%d)\n", path, mode); 
#endif
  res = docmd(1, path, strlen(path));
#ifdef FS_DEBUG
  printf("res = %d\n", res); 
#endif
  if(res>=0 && res<=MAXFD)
    readpos[res]=0;
  else
    res = -1;
  return res;
}

EXTERN_C int close(int fd)
{
  int res;
#ifdef FS_DEBUG
  printf("close(%d)\n", fd);
#endif
  res = docmd(3, &fd, 4);
#ifdef FS_DEBUG
  printf("res = %d\n", res);
#endif
  if(res>=0 && fd>=0 && fd<=MAXFD)
    readpos[fd]=-1;
  return res;
}

EXTERN_C int read(int fd, void *buf, unsigned int len)
{
  struct { int fd, pos, len; } cmd;
  int res, tot=0;
#ifdef FS_DEBUG
  printf("read(%d,%p,%d)\n", fd, buf, len);
#endif
  if(fd<0 || fd>MAXFD)
    return -1;
  while(len > 1024) {
    res = read(fd, buf, 1024);
    if(res <= 0)
      return (res>=0? res+tot : res);
    buf = ((char *)buf)+res;
    len -= res;
    tot += res;
  }
  cmd.fd = fd;
  cmd.pos = readpos[fd];
  cmd.len = len;
  res = docmd(2, &cmd, 12);
#ifdef FS_DEBUG
  printf("res = %d\n", res);
#endif
  if(res > 0) {
    readpos[fd] += res;
    memcpy(buf, replybuf, res);
  }
  return (res>=0? res+tot : res);
}

EXTERN_C int pread(int fd, void *buf, unsigned int nbyte, long offset)
{
  struct { int fd, pos, len; } cmd;
  int res, tot=0;
#ifdef FS_DEBUG
  printf("pread(%d,%p,%d,%d)\n", fd, buf, nbyte, offset);
#endif
  if(fd<0 || fd>MAXFD)
    return -1;
  while(nbyte > 1024) {
    res = pread(fd, buf, 1024, offset);
    if(res <= 0)
      return (res>=0? res+tot : res);
    buf = ((char *)buf)+res;
    nbyte -= res;
    offset += res;
    tot += res;
  }
  cmd.fd = fd;
  cmd.pos = offset;
  cmd.len = nbyte;
  res = docmd(2, &cmd, 12);
#ifdef FS_DEBUG
  printf("res = %d\n", res);
#endif
  if(res > 0)
    memcpy(buf, replybuf, res);
  return (res>=0? res+tot : res);
}

EXTERN_C long lseek(int fd, long pos, int whence)
{
#ifdef FS_DEBUG
  printf("lseek(%d,%d,%d)\n", fd, pos, whence); 
#endif
  if(fd<0 || fd>MAXFD || readpos[fd]<0)
    return -1;
  switch(whence) {
  case SEEK_SET:
    readpos[fd] = pos;
    break;
  case SEEK_CUR:
    readpos[fd] += pos;
    break;
  case SEEK_END:
    readpos[fd] = file_size(fd) + pos;
    break;
  default:
    return -1;
  }
#ifdef FS_DEBUG
  printf("res = %d\n", readpos[fd]); 
#endif
  return readpos[fd];
}

EXTERN_C int file_size(int fd)
{
  int res;
#ifdef FS_DEBUG
  printf("file_size(%d)\n", fd);
#endif
  if(fd<0 || fd>MAXFD || readpos[fd]<0)
    return -1;
  res = docmd(8, &fd, sizeof(fd));
#ifdef FS_DEBUG
  printf("res = %d\n", res); 
#endif
  return res;
}

EXTERN_C DIR *opendir(const char *path)
{
  int res;
  DIR *dirp = malloc(sizeof(DIR));
  if(dirp == NULL) return NULL;
#ifdef FS_DEBUG
  printf("opendir(%s)\n", path);
#endif
  res = docmd(4, path, strlen(path));
#ifdef FS_DEBUG
  printf("res = %d\n", res);
#endif
  if(res >= 0) {
    dirp->dd_fd = res;
    dirp->dd_loc = 0;
    return dirp;
  } else
    return 0;
}

EXTERN_C int closedir(DIR *dirp)
{
  int res;
#ifdef FS_DEBUG
  printf("closedir(%p)\n", dirp);
#endif
  res = docmd(5, &dirp->dd_fd, sizeof(dirp->dd_fd));
#ifdef FS_DEBUG
  printf("res = %d\n", res);
#endif
  free(dirp);
  return res;
}

static struct dirent g_entry;

EXTERN_C struct dirent *readdir(DIR *dirp)
{
  struct { int fd, pos; } cmd;
  int res;
#ifdef FS_DEBUG
  printf("readdir(%p)\n", dirp);
#endif
  cmd.fd = dirp->dd_fd;
  cmd.pos = dirp->dd_loc;
  res = docmd(6, &cmd, 8);
#ifdef FS_DEBUG
  printf("res = %d\n", res);
#endif
  if(res >= 0) {
    dirp->dd_loc++;
    memcpy(&g_entry, replybuf, replylen);
    ((char *)&g_entry)[replylen] = '\0';
    return &g_entry;
  } else
    return 0;
}

EXTERN_C int chdir(const char *path)
{
  int res;
#ifdef FS_DEBUG
  printf("chdir(%s)\n", path);
#endif
  res = docmd(7, path, strlen(path));
#ifdef FS_DEBUG
  printf("res = %d\n", res);
#endif
  return res;
}

EXTERN_C void cdfs_init()
{
  register unsigned long p, x;

  lwip_init();
  cmd_conn = netconn_new(NETCONN_UDP);
  netconn_bind(cmd_conn, IP_ADDR_ANY, 1449);
  netconn_connect(cmd_conn, IP_ADDR_BROADCAST, 1451);

  *((volatile unsigned long *)0xa05f74e4) = 0x1fffff;
  for(p=0; p<0x200000/4; p++)
    x = ((volatile unsigned long *)0xa0000000)[p];
  gdGdcInitSystem();  
}

EXTERN_C void cdfs_reinit()
{
}

EXTERN_C int play_cdda_tracks(int start, int stop, int reps)
{
  return -1;
}

EXTERN_C int play_cdda_sectors(int start, int stop, int reps)
{
  return -1;
}

EXTERN_C int stop_cdda(void)
{
  return -1;
}

EXTERN_C struct TOC *cdfs_gettoc()
{
  return NULL;
}

EXTERN_C int cdfs_diskchanges(void)
{
  return 0;
}

#endif
