#ifndef _RONIN_MATRIX_H
#define _RONIN_MATRIX_H been_here_before

#include "common.h"

START_EXTERN_C
extern void clear_matrix();
extern void ortho_matrix();
extern void load_matrix(const float (*matrix)[4][4]);
extern void save_matrix(float (*matrix)[4][4]);
extern void apply_matrix(const float (*matrix)[4][4]);
extern void transform_coords(const float (*src)[3], float (*dest)[3], int n);
extern void ta_commit_vertex(const void *list, float x, float y, float z);
END_EXTERN_C

#endif //_RONIN_MATRIX_H
