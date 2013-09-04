#ifndef _RONIN_SINCOS_RROOT_H
#define _RONIN_SINCOS_RROOT_H been_here_before

/* Macros to support the undocumented sh4 instructions FSCA and FSRRA */

/* Uses the undocumented sh4 instruction FSCA to calculate sin and cos
   for r_ and put the result in sin_ and cos_ respectively. r_ is an
   unsigned 16bit int (the to 16 bits are ignored), where 65565 is one
   whole circle. */
#define SINCOS(r_, sin_, cos_) \
  __asm__("lds %2,fpul; .word 0xf0fd; fmov fr0,%0; fmov fr1,%1" \
          : "=f" (sin_), "=f" (cos_) : "r" (r_) : "fpul", "fr0", "fr1")

/* Uses the undocumented sh4 instruction FSRRA to calculate
   1/sqrt(_src) and put the result in _dst. */
#define RROOT(src_, dst_) \
  __asm__("fmov %1,fr0; .word 0xf07d; fmov fr0,%0" \
          : "=f" (dst_) : "f" (src_) : "fr0")

#endif /* _RONIN_SINCOS_RROOT_H */
