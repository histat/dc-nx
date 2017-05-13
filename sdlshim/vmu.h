#ifndef _VMU_H
#define _VMU_H

bool vmfile_search(const char *fname, int *vm);
bool vmfile_exists(const char *fname);
bool save_to_vmu(int unit, const char *filename, const char *buf, int buf_len);
bool load_from_vmu(int unit, const char *filename, char *buf, int *buf_len);
bool delete_file_vmu(int unit, const char *filename);
bool rename_vmu_file(int unit, const char *oldpath, const char *newpath);

#endif

