#ifndef _SDFS_H_
#define _SDFS_H_

#include <ronin/common.h>

#include <fcntl.h>

#ifndef MAX_OPEN_FILES
#define MAX_OPEN_FILES 64
#endif

#define O_DIR    4

START_EXTERN_C
int sdfs_init(void);
int sdfs_exit(void);
END_EXTERN_C

#endif

