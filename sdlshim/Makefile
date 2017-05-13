ronindir = ../libronin

AS = sh-elf-as -little
CC = sh-elf-gcc -ml -m4-single-only
CXX = sh-elf-g++ -ml -m4-single-only
LD = sh-elf-ld -EL

RM	= rm -f
MKDIR   = mkdir

DEFINES  = -DUSE_ARM
INCLUDES = -I$(ronindir)/include -ISDL -I. -I../libsd
CFLAGS   = -Wall -Os $(INCLUDES) $(DEFINES) -Wreturn-type -Wunused-variable -Wno-multichar
CXXFLAGS = $(CFLAGS) -fno-exceptions
LDFLAGS = -Wl,-Ttext,0x8c010000 -nostartfiles $(ronindir)/lib/crt0.o
LIBS	= -L$(ronindir)/lib -lronin -lz -lm

ASSRC := asm.s
CSRC := 

CPPSRC := \
	main.cpp ronin.cpp dcevent.cpp console.cpp file.cpp \
	vmu.cpp vm_file.cpp \
	SDL/init.cpp SDL/screen.cpp \
	SDL/bmploader.cpp SDL/event.cpp SDL/audio.cpp SDL/misc.cpp \
	common/misc.cpp  \
	sound/sslib.cpp sound/org.cpp sound/pxt.cpp sound/cdda.cpp



ASOBJ     = $(patsubst %.s, %.o, $(ASSRC))
COBJ      = $(patsubst %.c, %.o, $(CSRC)) 
CPPOBJ    = $(patsubst %.cpp, %.o, $(CPPSRC))

OBJS = $(CPPOBJ) $(ASOBJ) $(COBJ)

all : sdlshim.elf

sdlshim.elf: $(OBJS) 
	$(CXX) -o $@ $(LDFLAGS) $(OBJS) $(LIBS) 

sdlshim.strip.elf: sdlshim.elf
	sh-elf-strip -o $@ $<

run: sdlshim.strip.elf
	ipupload.pike < $<

dump: sdlshim.elf
	sh-elf-objdump -D $< > dump

clean:
	$(RM) $(OBJS) sdlshim.elf sdlshim.strip.elf

