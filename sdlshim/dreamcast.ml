
<DEFAULT
COMPILE=sh-elf-gcc -ml -m4-single-only -fno-exceptions -I/usr/local/ronin/include -D__SDCARD__ -ISDL -Os -c %SRCFILE% -Wreturn-type -Wunused-variable -Wno-multichar -o %MODULE%.%OBJ_EXT%
LPREFIX=sh-elf-gcc -ml -m4-single-only -Wl,-Ttext,0x8c010000 -nostartfiles /usr/local/ronin/lib/crt0.o -o %OUTPUT%
LSUFFIX=-lstdc++ -L../lib -lsd -L/usr/local/ronin/lib -lronin-sd -lz -lm
OBJ_EXT=o
OUTPUT=sdlshim

<ASM
COMPILE=sh-elf-as -little %SRCFILE% -o %MODULE%.%OBJ_EXT%
NOPARSE=1

<NOPARSE
NOPARSE=1

>>
main.cpp
support.cpp
ronin.cpp
console.cpp

SDL/init.cpp
SDL/screen.cpp
SDL/bmploader.cpp
SDL/event.cpp
SDL/audio.cpp
SDL/misc.cpp

asm.s : ASM

common/misc.cpp
common/DBuffer.cpp
common/BList.cpp

dcevent.cpp

../sdlshim/sdfs.c
<<
