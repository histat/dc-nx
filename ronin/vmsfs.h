#ifndef _RONIN_VMSFS_H
#define _RONIN_VMSFS_H been_here_before
/*
 * VMS fs
 */
#include "common.h"

struct timestamp {
  int year;   /* 0000-9999 */
  int month;  /* 1-12      */
  int day;    /* 1-31      */
  int hour;   /* 0-23      */
  int minute; /* 0-59      */
  int second; /* 0-59      */
  int wkday;  /* 0-6       */
};

struct vmsinfo {
  int port;
  int dev;
  int pt;
  unsigned long func;

  int partitions;
  int blocksz;
  int writecnt;
  int readcnt;
  int removable;

  int root_loc;
  int fat_loc;
  int fat_size;
  int dir_loc;
  int dir_size;
  int icon_shape;
  int num_blocks;
};

struct superblock {
  unsigned char root[8192];
  unsigned char fat[8192];
  int root_modified;
  int fat_modified;
  struct vmsinfo *info;
};

struct dir_iterator {
  unsigned char blk[8192];
  unsigned int this_blk, next_blk;
  int dcnt, blks_left;
  struct superblock *super;
};

struct dir_entry {
  unsigned char entry[0x20];
  unsigned int dblk;
  int dpos;
  struct dir_iterator *dir;
};

struct vms_file_header {
  char shortdesc[16];
  char longdesc[32];
  char id[16];
  unsigned short numicons;
  unsigned short animspeed;
  unsigned short eyecatchtype;
  unsigned short crc;
  unsigned long filesize;
  unsigned long reserved[5];
  unsigned short palette[16];
};

struct vms_file {
  int loc0, blks;
  int loc, offs, left;
  unsigned int size;
  struct superblock *super;
  struct vms_file_header header;
  unsigned char blk[8192];
};

START_EXTERN_C
char *vmsfs_describe_error();
void vmsfs_timestamp_to_bcd(unsigned char *bcd, const struct timestamp *tstamp);
void vmsfs_timestamp_from_bcd(struct timestamp *tstamp, const unsigned char *bcd);
int vmsfs_check_unit(int unit, int part, struct vmsinfo *info);
int vmsfs_beep(struct vmsinfo *info, int on);
int vmsfs_read_block(struct vmsinfo *info, unsigned int blk, unsigned char *ptr);
int vmsfs_write_block(struct vmsinfo *info, unsigned int blk, unsigned char *ptr);
int vmsfs_get_superblock(struct vmsinfo *info, struct superblock *s);
int vmsfs_sync_superblock(struct superblock *s);
unsigned int vmsfs_get_fat(struct superblock *s, unsigned int n);
void vmsfs_set_fat(struct superblock *s, unsigned int n, unsigned int l);
int vmsfs_count_free(struct superblock *s);
int vmsfs_find_free_block(struct superblock *s);
void vmsfs_open_dir(struct superblock *s, struct dir_iterator *i);
int vmsfs_next_dir_entry(struct dir_iterator *i, struct dir_entry *d);
int vmsfs_next_named_dir_entry(struct dir_iterator *i, struct dir_entry *d, const char *name);
int vmsfs_next_empty_dir_entry(struct dir_iterator *i, struct dir_entry *d);
int vmsfs_write_dir_entry(struct dir_entry *d);
int vmsfs_open_file(struct superblock *super, const char *name,
                    struct vms_file *file);
int vmsfs_read_file(struct vms_file *file, unsigned char *buf,
                    unsigned int cnt);
int vmsfs_create_file(struct superblock *super, const char *name,
                      struct vms_file_header *header,
                      const void *icons, const void *eyecatch,
                      const void *data, unsigned long datasize,
                      const struct timestamp *tstamp);
int vmsfs_delete_file(struct superblock *super, const char *name);
extern int vmsfs_errno;
END_EXTERN_C


#endif //_RONIN_VMSFS_H
