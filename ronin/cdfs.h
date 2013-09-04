#ifndef _RONIN_CDFS_H
#define _RONIN_CDFS_H been_here_before
/*
 * ISO 9660 fs 
 */
#include "common.h"

struct TOC {
  unsigned int entry[99];
  unsigned int first, last;
  unsigned int dunno;
};

#define TOC_LBA(n) ((n)&0x00ffffff)
#define TOC_ADR(n) (((n)&0x0f000000)>>24)
#define TOC_CTRL(n) (((n)&0xf0000000)>>28)
#define TOC_TRACK(n) (((n)&0x00ff0000)>>16)

typedef struct {
  int     dd_fd;
  int     dd_loc;
  int     dd_size;
  char    *dd_buf;
} DIR;

#define MAX_OPEN_FILES 64

typedef struct dirent {
  int      d_size;
  char     d_name[256];
} dirent_t;

#define O_RDONLY 0
#define O_DIR    4

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

START_EXTERN_C
int open(const char *path, int oflag, ...);
int close(int fd);
int pread(int fd, void *buf, unsigned int nbyte, long offset);
int read(int fd, void *buf, unsigned int nbyte);
long int lseek(int fd, long int offset, int whence);
DIR *opendir(const char *dirname);
int closedir(DIR *dirp);
int file_size( int fd );
int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **res);
struct dirent *readdir(DIR *dirp);
int chdir(const char *path);
void cdfs_init(void);
void cdfs_reinit(void);
int cdfs_diskchanges(void);

int play_cdda_tracks(int start, int stop, int reps);
int play_cdda_sectors(int start, int stop, int reps);
int stop_cdda(void);

struct TOC *cdfs_gettoc(void);
END_EXTERN_C

#endif //_RONIN_CDFS_H
