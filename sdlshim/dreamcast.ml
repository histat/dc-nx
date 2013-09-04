
<DEFAULT
COMPILE=sh-elf-gcc -ml -m4-single-only -fno-exceptions -D__SDCARD__ -I../ronin/include -ISDL -Os -c %SRCFILE% -Wreturn-type -Wunused-variable -Wno-multichar -o %MODULE%.%OBJ_EXT%
LPREFIX=sh-elf-gcc -ml -m4-single-only -Wl,-Ttext,0x8c010000 -nostartfiles ../ronin/lib/crt0.o -o %OUTPUT%
LSUFFIX=-lstdc++ -L../lib -lsd -L../ronin/lib -lronin-sd -lz -lm
OBJ_EXT=o
OUTPUT=sdlshim.elf

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
SDL/misc.cpp

asm.s : ASM

common/misc.cpp
common/DBuffer.cpp
common/BList.cpp

sound/sslib.cpp
sound/org.cpp
sound/pxt.cpp

vmu.cpp
dcevent.cpp
scrnsave.cpp
sdfs.c
<<
SDL/audio.cpp






