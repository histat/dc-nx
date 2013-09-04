#include <string.h> //FIXME: External dependecy.
#include <ctype.h>
#include "cdfs.h"
#include "gddrive.h"
#include "dc_time.h"
#include "notlibc.h"

#define ERR_SYSERR   -1
#define ERR_DIRERR   -2
#define ERR_NOFILE   -3
#define ERR_PARAM    -4
#define ERR_NUMFILES -5
#define ERR_NODISK   -6
#define ERR_DISKCHG  -7

#define NUM_BUFFERS  16

static struct TOC *current_toc = NULL;

#define TOC_LBA(n) ((n)&0x00ffffff)
#define TOC_ADR(n) (((n)&0x0f000000)>>24)
#define TOC_CTRL(n) (((n)&0xf0000000)>>28)
#define TOC_TRACK(n) (((n)&0x00ff0000)>>16)

static unsigned int sector_buffer[NUM_BUFFERS][2048/4];
static unsigned int dir_buffer[2048/4];
static int drive_inited = -1;
static int secbuf_secs[NUM_BUFFERS];
static int cwd_sec, cwd_len;
static int discchange_count;

/*
 * libc like support
 */

/* static int memcmp(const void *p1, const void *p2, unsigned int size) */
/* { */
/*   const unsigned char *m1 = p1, *m2 = p2; */
/*   while(size--) */
/*     if(*m1++ != *m2++) */
/*       return m2[-1]-m1[-1]; */
/*   return 0; */
/* } */

/* static void *memcpy(void *p1, const void *p2, unsigned int size) */
/* { */
/*   unsigned char *m1 = p1; */
/*   const unsigned char *m2 = p2; */
/*   while(size--) */
/*     *m1++ = *m2++; */
/*   return p1; */
/* } */

static char *strchr0(const char *s, int c)
{
  while(*s!=c)
    if(!*s++)
      return NULL;
  return (char *)s;
}



/*
 * GDROM access
 */

static int gdfs_errno_to_errno(int n)
{
 switch(n) {
   case 2:
     drive_inited = -1;
     current_toc = NULL;
     return ERR_NODISK;
   case 6:
     discchange_count++;
     drive_inited = -1;
     current_toc = NULL;
     return ERR_DISKCHG;
   default:
     return ERR_SYSERR;
  }
}

static int send_cmd(int cmd, void *param)
{
  return gdGdcReqCmd(cmd, param);
}

static int check_cmd(int f)
{
  int blah[4];
  int n;
  gdGdcExecServer();
  if((n = gdGdcGetCmdStat(f, blah))==1)
    return 0;
  if(n == 2)
    return 1;
  else return gdfs_errno_to_errno(blah[0]);
}

static int wait_cmd(int f)
{
  int n;
  while(!(n = check_cmd(f)));
  return (n>0? 0 : n);
}

static int exec_cmd(int cmd, void *param)
{
  int f = send_cmd(cmd, param);
  return wait_cmd(f);
}

int cdfs_diskchanges()
{
  return discchange_count;
}

static int init_drive()
{
  int i, r=0;
  unsigned int param[4];
  int cdxa;

  for(i=0; i<8; i++)
    if(!(r = exec_cmd(24, NULL)))
      break;
  if(r)
    return r;

  gdGdcGetDrvStat(param);

  cdxa = (param[1] == 32);

  param[0] = 0; /* set data type */
  param[1] = 8192;
  param[2] = (cdxa? 2048 : 1024); /* mode 1/2 */
  param[3] = 2048; /* sector size */

  if(gdGdcChangeDataType(param)<0)
    return ERR_SYSERR;

  drive_inited = 1;

  return 0;
}

static int read_toc(struct TOC *toc, int session)
{
  struct { int session; void *buffer; } param;
  param.session = session;
  param.buffer = toc;
  return exec_cmd(19, &param);
}

static int read_sectors(char *buf, int sec, int num)
{
  struct { int sec, num; void *buffer; int dunno; } param;
  param.sec = sec;
  param.num = num;
  param.buffer = buf;
  param.dunno = 0;
  return exec_cmd(16, &param);
}

static int read_sectors_async(char *buf, int sec, int num)
{
  struct { int sec, num; void *buffer; int dunno; } param;
  param.sec = sec;
  param.num = num;
  param.buffer = buf;
  param.dunno = 0;
  return send_cmd(16, &param);
}

static int read_cached_sector(unsigned char **buf, int sec)
{
  int i;
  static int robin = 1;
  if(drive_inited>0 && sec)
    for(i=1; i<NUM_BUFFERS; i++)
      if(secbuf_secs[i] == sec) {
	*buf = (unsigned char *)sector_buffer[i];
	return 0;
      }
  secbuf_secs[robin] = 0;
  if((i = read_sectors((char *)sector_buffer[robin], sec, 1)))
    return i;
  *buf = (unsigned char *)sector_buffer[robin];
  secbuf_secs[robin++] = sec;
  if(robin >= NUM_BUFFERS)
    robin = 1;
  return 0;
}

/*
 * CDDA
 */

int play_cdda_tracks(int start, int stop, int reps)
{
  struct { int start, stop, reps, dunno; } param;
  param.start = start;
  param.stop = stop;
  param.reps = reps;
  param.dunno = 0;
  return exec_cmd(20, (void *)&param);
}

int play_cdda_sectors(int start, int stop, int reps)
{
  struct { int start, stop, reps, dunno; } param;
  param.start = start;
  param.stop = stop;
  param.reps = reps;
  param.dunno = 0;
  return exec_cmd(21, (void *)&param);
}

int stop_cdda()
{
  return exec_cmd(22, NULL);
}

/*
 * ISO9660 support functions
 */

static int ntohlp(unsigned char *ptr)
{
  return (ptr[0]<<24)|(ptr[1]<<16)|(ptr[2]<<8)|ptr[3];
}

static int fncompare(const char *fn1, int fn1len, const char *fn2, int fn2len)
{
  while(fn2len--)
    if(!fn1len--)
      return *fn2 == ';';
    else if(toupper(*fn1++) != toupper(*fn2++))
      return 0;
  return fn1len == 0;
}


/* 
 * Low file I/O
 */

static unsigned int find_datatrack(struct TOC *toc)
{
  int i, first, last;
  first = TOC_TRACK(toc->first);
  last = TOC_TRACK(toc->last);
  if(first < 1 || last > 99 || first > last)
    return 0;
  for(i=last; i>=first; --i)
    if(TOC_CTRL(toc->entry[i-1])&4)
      return TOC_LBA(toc->entry[i-1]);
  return 0;
}

static int find_root(unsigned int *psec, unsigned int *plen)
{
  static struct TOC toc;
  static unsigned int saved_sec, saved_len;
  int r;
  unsigned int sec;

  if(drive_inited>0 && saved_sec) {
    *psec = saved_sec;
    *plen = saved_len;
    return 0;
  }
  saved_sec = saved_len = 0;
  cwd_sec = cwd_len = 0;
  memset(secbuf_secs, 0, sizeof(secbuf_secs));
  if((r=init_drive())!=0)
    return r;
  if((r=read_toc(&toc, 0))!=0)
    return r;
  current_toc = &toc;
  if(!(sec = find_datatrack(&toc)))
    return ERR_DIRERR;
  if((r=read_sectors((char *)sector_buffer[0], sec+16, 1))!=0)
    return r;
  if(memcmp((char *)sector_buffer[0], "\001CD001", 6))
    return ERR_DIRERR;
  *psec = saved_sec = ntohlp(((char *)sector_buffer[0])+156+6) + 150;
  *plen = saved_len = ntohlp(((char *)sector_buffer[0])+156+14);
  return 0;
}

static int low_find(unsigned int sec, unsigned int dirlen, int isdir,
		    unsigned int *psec, unsigned int *plen,
		    const char *fname, int fnlen)
{
  isdir = (isdir? 2 : 0);
  while(dirlen>0) {
    int r, i;
    unsigned char *rec;
    if((r=read_cached_sector(&rec, sec))!=0)
      return r;
    for(i=0; i<2048 && i<dirlen && rec[0] != 0; i += rec[0], rec += rec[0]) {
      if((rec[25]&2) == isdir && fncompare(fname, fnlen, rec+33, rec[32])) {
	*psec = ntohlp(rec+6)+150;
	*plen = ntohlp(rec+14);
	return 0;
      }
    }
    sec++;
    dirlen -= 2048;
  }
  return ERR_NOFILE;
}

/* File I/O */

#define MIN_FD 3

static struct {
  unsigned int sec0;
  unsigned int loc;
  unsigned int len;
  int async, disc_gen;
} fh[MAX_OPEN_FILES];

int open(const char *path, int oflag, ...)
{
  int fd, r;
  unsigned int sec, len;
  char *p;

  for(fd=0; fd<MAX_OPEN_FILES; fd++)
    if(fh[fd].sec0 == 0)
      break;
  if(fd>=MAX_OPEN_FILES)
    return ERR_NUMFILES;
  if(drive_inited>0 && cwd_sec > 0 && *path!='/') {
    sec = cwd_sec;
    len = cwd_len;
  } else
    if((r=find_root(&sec, &len)))
      return r;
  while((p = strchr0(path, '/'))) {
    if(p != path)
      if((r = low_find(sec, len, 1, &sec, &len, path, p-path)))
	return r;
    path = p+1;
  }
  if(*path)
    if((r = low_find(sec, len, oflag&O_DIR, &sec, &len, path,
		     strchr0(path, '\0')-path)))
      return r;
    else ;
  else
    if(!(oflag&O_DIR))
      return ERR_NOFILE;
  fh[fd].sec0 = sec;
  fh[fd].loc = 0;
  fh[fd].len = len;
  fh[fd].async = -1;
  fh[fd].disc_gen = discchange_count;
  return fd+MIN_FD;
}

int close(int fd)
{
  if(fd<MIN_FD || fd>=MAX_OPEN_FILES+MIN_FD)
    return ERR_PARAM;
  fh[fd-MIN_FD].sec0 = 0;
  return 0;
}

int file_size( int fd ) /* hm */
{
  if(fd<MIN_FD || fd>=MAX_OPEN_FILES+MIN_FD) return ERR_PARAM;
  return fh[fd-MIN_FD].len;
}

int pread(int fd, void *buf, unsigned int nbyte, long offset)
{
  int r, t;
  if(fd<MIN_FD || fd>=MAX_OPEN_FILES+MIN_FD)
    return ERR_PARAM;
  if(fh[fd-MIN_FD].disc_gen != discchange_count)
    return ERR_DISKCHG;
  if(offset>=fh[fd-MIN_FD].len)
    return 0;
  if(offset+nbyte > fh[fd-MIN_FD].len)
    nbyte = fh[fd-MIN_FD].len - offset;
  if(nbyte>=2048 && !(offset & 2047))
    if((r = read_sectors(buf, fh[fd-MIN_FD].sec0 + (offset>>11), nbyte>>11)))
      return r;
    else {
      t = nbyte & ~2047;;
      buf = ((char *)buf) + t;
      offset += t;
      nbyte &= 2047;
    }
  else
    t = 0;
  if(!nbyte)
    return t;
  if((offset & 2047)+nbyte > 2048) {
    if((r = pread(fd, buf, 2048-(offset & 2047), offset))<0)
      return r;
    else {
      t += r;
      buf = ((char *)buf) + r;
      offset += r;
      nbyte -= r;
    }
    if((r = pread(fd, buf, nbyte, offset))<0)
      return r;
    else
      t += r;
  } else {
    unsigned char *csec;
    if((r = read_cached_sector(&csec, fh[fd-MIN_FD].sec0+(offset>>11))))
      return r;
    memcpy(buf, csec+(offset&2047), nbyte);
    t += nbyte;
  }
  return t;
}

int read(int fd, void *buf, unsigned int nbyte)
{
  if(fd<MIN_FD || fd>=MAX_OPEN_FILES+MIN_FD)
    return ERR_PARAM;
  else {
    int r = pread(fd, buf, nbyte, fh[fd-MIN_FD].loc);
    if(r>0)
      fh[fd-MIN_FD].loc += r;
    return r;
  };
}

static int read_cached(int fd, void *buf, unsigned int nbyte)
{
  if(fd<MIN_FD || fd>=MAX_OPEN_FILES+MIN_FD || nbyte != 2048 ||
     (fh[fd-MIN_FD].loc & 2047))
    return ERR_PARAM;
  else if(fh[fd-MIN_FD].disc_gen != discchange_count)
    return ERR_DISKCHG;
  else if(fh[fd-MIN_FD].loc>=fh[fd-MIN_FD].len)
    return 0;
  else {
    unsigned char *sec;
    int r = read_cached_sector(&sec, fh[fd-MIN_FD].sec0+
			       (fh[fd-MIN_FD].loc>>11));
    if(r>=0) {
      memcpy(buf, sec, nbyte);
      fh[fd-MIN_FD].loc += nbyte;
      return nbyte;
    }
    return r;
  };
}

long int lseek(int fd, long int offset, int whence)
{
  if(fd<MIN_FD || fd>=MAX_OPEN_FILES+MIN_FD)
    return ERR_PARAM;
  switch(whence) {
   case SEEK_SET:
     return fh[fd-MIN_FD].loc = offset;
   case SEEK_CUR:
     return fh[fd-MIN_FD].loc += offset;
   case SEEK_END:
     return fh[fd-MIN_FD].loc = fh[fd-MIN_FD].len + offset;
   default:
     return ERR_PARAM;
  }
}

/* Asynch I/O */

int asynch_read(int fd, void *buf, unsigned int nbyte)
{
  if(fd<MIN_FD || fd>=MAX_OPEN_FILES+MIN_FD ||
     (fh[fd-MIN_FD].loc & 2047) || (nbyte & 2047))
    return ERR_PARAM;
  fd -= MIN_FD;
  if(fh[fd].disc_gen != discchange_count)
    return ERR_DISKCHG;
  if(fh[fd].loc + nbyte > ((fh[fd].len+2047)&~2047))
    return ERR_PARAM;
  if( fh[fd].async != -1 )
    return ERR_PARAM;
  fh[fd].async = read_sectors_async(buf, fh[fd].sec0 + (fh[fd].loc>>11),
				    nbyte>>11);
  fh[fd].loc += nbyte;
  return 0;
}

int async_check(int fd)
{
  int n;
  if(fd<MIN_FD || fd>=MAX_OPEN_FILES+MIN_FD || fh[fd-MIN_FD].async == -1)
    return ERR_PARAM;
  n = check_cmd(fh[fd-MIN_FD].async);
  if(n)
    fh[fd-MIN_FD].async = -1;
  return n;
}

int async_wait(int fd)
{
  int n;
  if(fd<MIN_FD || fd>=MAX_OPEN_FILES+MIN_FD || fh[fd-MIN_FD].async == -1)
    return ERR_PARAM;
  n = wait_cmd(fh[fd-MIN_FD].async);
  if(n>0)
    fh[fd-MIN_FD].async = -1;
  return n;
}

/* Dir I/O */

static struct dirent g_dirent;

DIR *opendir(const char *dirname)
{
  DIR *dirp = malloc(sizeof(DIR));
  if(dirp == NULL || (dirp->dd_fd = open(dirname, O_DIR|O_RDONLY)) < 0)
    return NULL;
  dirp->dd_loc = 0;
  dirp->dd_size = 0;
  dirp->dd_buf = (char *)dir_buffer;
  return dirp;
}

int closedir(DIR *dirp)
{
  if(dirp) {
    int res = close(dirp->dd_fd);
    free(dirp);
    return res;
  } else
    return ERR_PARAM;
}

int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **res)
{
  int l, r=0;
  unsigned char *rec;

  if(dirp == NULL || dirp->dd_fd<0)
    r = ERR_PARAM;
  else {
    do {
      while(dirp->dd_loc >= dirp->dd_size ||
	    (l=((unsigned char *)dirp->dd_buf)[dirp->dd_loc]) == 0 ||
	    dirp->dd_loc + l > dirp->dd_size) {
	/* Need to read more dir data */
	if((r = read_cached(dirp->dd_fd, dirp->dd_buf, 2048))<=0) {
	  *res = NULL;
	  return (r? r : ERR_NOFILE);
	}
	dirp->dd_loc = 0;
	dirp->dd_size = r;
	r = 0;
      }
      rec = dirp->dd_buf+dirp->dd_loc;
      dirp->dd_loc += l;
      memcpy(entry->d_name, rec+33, rec[32]);
      entry->d_name[rec[32]] = '\0';
      entry->d_size = ((rec[25]&2)? -1 : ntohlp(rec+14));
      if((rec = strchr0(entry->d_name, ';')))
	*rec = '\0';
    } while(entry->d_name[0]==0 || entry->d_name[0]==1);
  }
  if(r) {
    *res = NULL;
    return r;
  } else {
    *res = entry;
    return 0;
  }
}

struct dirent *readdir(DIR *dirp)
{
  struct dirent *res;
  readdir_r(dirp, &g_dirent, &res);
  return res;
}

int chdir(const char *path)
{
  int fd = open(path, O_DIR|O_RDONLY);
  if(fd<0)
    return fd;
  cwd_sec = fh[fd-MIN_FD].sec0;
  cwd_len = fh[fd-MIN_FD].len;
  close(fd);
  return 0;
}

/* Init function */

void cdfs_init()
{
  register unsigned long p, x;

  usleep(1000);

  /* Reactivate GD-ROM drive */

  *((volatile unsigned long *)0xa05f74e4) = 0x1fffff;
  for(p=0; p<0x200000/4; p++)
    x = ((volatile unsigned long *)0xa0000000)[p];

  /* Reset GD system functions */

  gdGdcInitSystem();
}

void cdfs_reinit()
{
  drive_inited = -1;
  current_toc = NULL;
}

struct TOC *cdfs_gettoc()
{
  return current_toc;
}
