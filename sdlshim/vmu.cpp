#include <ronin/ronin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include "vmu.fdh"
#include "cave_icon.h"

#define MAX_VMU_SIZE (128 * 1024)

extern char caption[];

extern int vmu_present[4];

static int vm_file;


#define check_audio() update_audio()

bool vmfile_search(const char *fname, int *vm)
{
	struct vmsinfo info;
	struct superblock super;
	struct vms_file file;

	bool vmu_avail[4*2];

	memset(vmu_avail, false, sizeof(vmu_avail));

	for (int i=0; i<4; i++) {
		if (vmu_present[i] & (1<<0)) {
			vmu_avail[i] = true;
		}
		if (vmu_present[i] & (1<<1)) {
			vmu_avail[i+4] = true;
		}
	}

	check_audio();

	for (int x=0; x<4; x++) {
		for (int y=0; y<2; y++) {
			
			if (vmu_avail[x+y*4]) {

				int res = x*6 + y + 1;
				
				if (vmsfs_check_unit(res, 0, &info))
					if (vmsfs_get_superblock(&info, &super))
						if (vmsfs_open_file(&super, fname, &file)) {
#ifndef NOSERIAL
							printf("%s Found on %c%d\n", fname, 'A'+res/6,res%6);
#endif
							check_audio();
							
							*vm = res;
						return true;
					}
			}
		}
	}

	check_audio();
	
	return false;
}

bool vmfile_exists(const char *fname)
{
	return vmfile_search(fname, &vm_file);
}

bool save_to_vmu(int unit, const char *filename, const char *buf, int buf_len)
{
	struct vms_file_header header;
	struct vmsinfo info;
	struct superblock super;
	struct vms_file file;
	int free_cnt;
	time_t long_time;
	struct tm *now_time;
	struct timestamp stamp;
	char shortdesc[16];
	char longdesc[32];
	unsigned char compressed_buf[MAX_VMU_SIZE];
	int compressed_len;

	memset(compressed_buf, 0, sizeof(compressed_buf));
	compressed_len = buf_len + 512;

	if (compress((Bytef*)compressed_buf, (uLongf*)&compressed_len,
				 (Bytef*)buf, buf_len) != Z_OK) {

		return false;
	}

	check_audio();
	
	sprintf(shortdesc,"%s %s",caption, "save");
	sprintf(longdesc,"%s", caption);

	if (!vmsfs_check_unit(unit, 0, &info)) {
		return false;
	}
	if (!vmsfs_get_superblock(&info, &super)) {
		return false;
	}
	free_cnt = vmsfs_count_free(&super);

	if (vmsfs_open_file(&super, filename, &file))
		free_cnt += file.blks;

	if (((128+512+compressed_len+511)/512) > free_cnt) {
		return false;
	}

	memset(&header, 0, sizeof(header));
	strncpy(header.shortdesc, shortdesc,sizeof(header.shortdesc));
	strncpy(header.longdesc, longdesc, sizeof(header.longdesc));
	strncpy(header.id, caption, sizeof(header.id));
	memcpy(header.palette, cave_icon, sizeof(header.palette));
	header.numicons = 1;

	time(&long_time);
	now_time = localtime(&long_time);
	stamp.year = now_time->tm_year + 1900;
	stamp.month = now_time->tm_mon + 1;
	stamp.wkday = now_time->tm_wday;
	stamp.day = now_time->tm_mday;
	stamp.hour = now_time->tm_hour;
	stamp.minute = now_time->tm_min;
	stamp.second = now_time->tm_sec;

	check_audio();
	
	vmsfs_beep(&info, 1);
	
	if (!vmsfs_create_file(&super, filename, &header, cave_icon+sizeof(header.palette), NULL, compressed_buf, compressed_len, &stamp)) {
		
		vmsfs_beep(&info, 0);
#ifndef NOSERIAL
		fprintf(stderr,"%s",vmsfs_describe_error());
#endif
		return false;
	}

	vmsfs_beep(&info, 0);

	check_audio();
	
	return true;
}

bool load_from_vmu(int unit, const char *filename, char *buf, int *buf_len)
{
	struct vmsinfo info;
	struct superblock super;
	struct vms_file file;
	unsigned char compressed_buf[MAX_VMU_SIZE];
	unsigned int compressed_len;

	if (!vmsfs_check_unit(unit, 0, &info)) {
		return false;
	}
	if (!vmsfs_get_superblock(&info, &super)) {
		return false;
	}
	if (!vmsfs_open_file(&super, filename, &file)) {
		return false;
	}

	check_audio();
	
	memset(compressed_buf, 0, sizeof(compressed_buf));
	compressed_len = file.size;
  
	if (!vmsfs_read_file(&file, (Bytef*)compressed_buf, compressed_len)) {

		return false;
	}

	if (!*buf_len)
		*buf_len = MAX_VMU_SIZE;

	if (uncompress((Bytef*)buf, (uLongf*)buf_len,(const Bytef*)compressed_buf, (uLong)compressed_len) != Z_OK) {

		return false;
	}

	check_audio();
	
	return true;
}

bool delete_file_vmu(int unit, const char *filename)
{
	struct vmsinfo info;
	struct superblock super;

	check_audio();
	
	if (!vmsfs_check_unit(unit, 0, &info)) {
		return false;
	}
	if (!vmsfs_get_superblock(&info, &super)) {
		return false;
	}
	if (!vmsfs_delete_file(&super, filename)) {
		return false;
	}

	check_audio();
	
	return true;
}

bool rename_vmu_file(const char *oldpath, const char *newpath)
{
	int vm;
	struct dir_iterator i;
	struct dir_entry d;

	struct vmsinfo info;
	struct superblock super;

	if (!vmfile_search(oldpath, &vm))
		return false;
	
	if (!vmsfs_check_unit(vm, 0, &info)) {
		return false;
	}
	if (!vmsfs_get_superblock(&info, &super)) {
		return false;
	}

	vmsfs_open_dir(&super, &i);

	int result = 0;
	
	while (vmsfs_next_dir_entry(&i, &d)) 
		if (d.entry[0] && !strncmp((const char *)d.entry+4, oldpath, 12)) {
			result = 1;
			break;
		}

	check_audio();
	
	if (!result)
		return false;

	char *dst = (char *)d.entry+4;
	memset(dst, 0, 12);
	strncpy(dst, newpath, 12);
	
#ifndef NOSERIAL
	printf("Rename: %s -> %s on %c%d\n", oldpath, newpath, 'A'+ vm/6, vm%6);
#endif
	
	return true;
}
