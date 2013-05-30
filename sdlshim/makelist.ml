
<DEFAULT
COMPILE=/opt/mingw32ce/bin/arm-mingw32ce-gcc -O2 -c %SRCFILE% -Wreturn-type -Wunused-variable -Wno-multichar -o %MODULE%.%OBJ_EXT%
LPREFIX=/opt/mingw32ce/bin/arm-mingw32ce-gcc -Wl,--enable-auto-import -o %OUTPUT%
LSUFFIX=-lstdc++
OBJ_EXT=o
OUTPUT=sdlshim.exe

<ASM
COMPILE=/opt/mingw32ce/bin/arm-mingw32ce-as -mcpu=xscale %SRCFILE% -o %MODULE%.%OBJ_EXT%
NOPARSE=1

<NOPARSE
NOPARSE=1

>>
main.cpp
support.cpp
gapi.cpp
console.cpp
file.cpp

SDL/init.cpp
SDL/screen.cpp
SDL/bmploader.cpp
SDL/event.cpp
SDL/audio.cpp
SDL/misc.cpp

asm.s : ASM
hacks.cpp

common/misc.cpp
common/DBuffer.cpp
common/BList.cpp
<<
