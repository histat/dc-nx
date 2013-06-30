#include <stdio.h>
#include <string.h>
#include "sdfs.h"
#include <ronin/dc_time.h>
#include <ronin/notlibc.h>
#include <ronin/report.h>
#include <ronin/serial.h>
#include "../lib/ff.h"
#include "dcsound.h"
#include "sdfs.fdh"


static FATFS Fatfs;
static FILINFO _fi;
//static DIR _dir;

#ifdef _USE_LFN
static char lfn[_MAX_LFN + 1];
#endif

#define MIN_FD 3

static struct {
	int used;
	FIL fil;
}fh[MAX_OPEN_FILES];

int sdfs_init(void) {

	int i;
	
	f_mount(0, &Fatfs);

#ifdef _USE_LFN	
	_fi.lfname = lfn;
	_fi.lfsize = sizeof(lfn);
#endif

	for(i=0; i<MAX_OPEN_FILES; i++) {
		fh[i].used = 0;
	}

	return 1;
}

int sdfs_exit(void) {

	f_mount(0, NULL);

	return 1;
}

int open(const char *path, int oflag, ...)
{
	FRESULT res;
	FIL *fp;
	int fd;

	int mode = 0;
	
	for(fd=0; fd<MAX_OPEN_FILES; fd++)
		if(fh[fd].used == 0)
			break;
	
	if(fd>=MAX_OPEN_FILES)
		return -1;

#ifndef NOSERIAL
	reportf("Open %s flag=0x%x\n", path, oflag);
#endif

	if(oflag & O_DIR) {
#ifndef NOSERIAL
		reportf("** O_DIR ignored\n");
#endif
		return -1;
	}
	else {

		fp = &fh[fd].fil;

		if (oflag & O_WRONLY)
			mode = FA_WRITE;
		else if (oflag & O_RDWR && !(oflag&(O_CREAT|O_TRUNC)))
			mode = FA_READ | FA_WRITE | FA_OPEN_ALWAYS;
		else
			mode = FA_READ | FA_OPEN_EXISTING;

		if ((oflag&(O_RDWR|O_WRONLY)) && (oflag&(O_CREAT|O_TRUNC)))
			mode |= FA_CREATE_ALWAYS;

		res = f_open(fp, path, mode);

		if (res != FR_OK) {
#ifndef NOSERIAL
			reportf("Open failed %s res=%d\n", path, res);
#endif
			return -1;
		}

		fh[fd].used = 1;
	}

	return fd+MIN_FD;
}


int close(int fd)
{
	FIL *fp;
	
	if(fd<MIN_FD || fd>=MAX_OPEN_FILES+MIN_FD)
		return -1;

	fp = &fh[fd-MIN_FD].fil;
  
	f_close (fp);

	fh[fd-MIN_FD].used = 0;

	return 0;
}

int read(int fd, void *ptr, unsigned int nbyte)
{
	FIL *fp;
	unsigned int readsize;
	
	if(fd<MIN_FD || fd>=MAX_OPEN_FILES+MIN_FD)
		return -1;

	fp = &fh[fd-MIN_FD].fil;

	if(f_read (fp, ptr, nbyte, &readsize) == FR_OK)
		return readsize;

	return 0;
}

long int lseek(int fd, long int offset, int whence)
{
	FIL *fp;
	long	ret;
	FRESULT res;

	if(fd<MIN_FD || fd>=MAX_OPEN_FILES+MIN_FD)
		return -1;

	fp = &fh[fd-MIN_FD].fil;

	ret = 0;
	
	switch (whence) {
	case 1:
		ret = f_tell(fp);
		break;
	case 2:
		ret = f_size(fp);
		break;
	}
	ret += offset;

#if 0	
	if (ret < 0) {
		ret = 0;
	} else if (ret > (long)f_size(fp)) {
		ret = f_size(fp);
	}
#endif

	res = f_lseek (fp, ret);

	if (res != FR_OK) {
		return -1;
	}

	return ret;
}


int write(int fd, const char *ptr, int len)
{
	FIL *fp;
	unsigned int writesize;
	int n=len;

	if(fd == 0) return -1;
	else if(fd == 1 || fd == 2) {
#ifndef NOSERIAL
		while(n-- > 0) serial_putc(*ptr++);
		serial_flush();
#endif
		return len;
	}
	
	if(fd>=MAX_OPEN_FILES+MIN_FD)
		return -1;

	fp = &fh[fd-MIN_FD].fil;
	
	if (f_write (fp, ptr, len, &writesize) == FR_OK)
		return(writesize);

	return 0;
}

int file_size( int fd )
{
	FIL *fp;

	if(fd<MIN_FD || fd>=MAX_OPEN_FILES+MIN_FD) return -1;
	fp = &fh[fd-MIN_FD].fil;
	return f_size(fp);
}

int creat(const char *path, mode_t mode)
{
	return open(path,O_WRONLY|O_TRUNC);
}


int unlink(const char *path)
{
	FRESULT res;
	
	res = f_unlink(path);
	return (res == FR_OK)? 0:-1;
}

int remove(const char *path)
{
	return unlink(path);
}

int rename(const char *oldpath, const char *newpath){

	FRESULT res;

	res = f_rename(oldpath, newpath);

	return (res == FR_OK)? 0:-1;
}

DIR *opendir(const char *dirname)
{
    return NULL;
}

int closedir(DIR *dirp)
{
    return -1;
}

int mkdir(const char *path, mode_t mode)
{
	FRESULT res;
#ifndef NOSERIAL
	reportf("%s: %s\n", __func__, path);
#endif

	res = f_mkdir(path);

	if(res != FR_OK)
		return -1;
	
	return 0;
}

int chdir(const char *path)
{
	FRESULT res;
	
	res = f_chdir(path);

	if(res != FR_OK)
		return -1;
	
	return 0;
}

