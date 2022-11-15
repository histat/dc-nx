#DEFINES  = -DUSE_ARM
INCLUDES = -I $(KOS_PORTS)/include/zlib -ISDL -I.
CFLAGS   = $(KOS_CFLAGS) $(INCLUDES) $(DEFINES) -Wreturn-type -Wunused-variable -Wno-multichar
CXXFLAGS = $(KOS_CXXFLAGS)
LDFLAGS = -s
LIBS	= -lz -lm -lstdc++

ASSRC := asm.s
CSRC := audio.c aica.bin.c libpspvram/valloc.c watchdog.c profile.c

CPPSRC := \
	main.cpp shim.o console.cpp file.cpp \
	vmu.cpp vm_file.cpp \
	SDL/init.cpp SDL/screen.cpp \
	SDL/bmploader.cpp SDL/event.cpp SDL/audio.cpp SDL/misc.cpp \
	common/misc.cpp  common/BList.cpp \
	sound/sslib.cpp sound/org.cpp sound/pxt.cpp



ASOBJ     = $(patsubst %.s, %.o, $(ASSRC))
COBJ      = $(patsubst %.c, %.o, $(CSRC)) 
CPPOBJ    = $(patsubst %.cpp, %.o, $(CPPSRC))

OBJS = $(CPPOBJ) $(ASOBJ) $(COBJ)

TARGET = sdlshim.elf

all : sdlshim

sdlshim: $(OBJS)
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) $(LDFLAGS) -o $(TARGET) $(KOS_START) \
	$(OBJS) $(LIBS) $(KOS_LIBS)

include $(KOS_BASE)/Makefile.rules

NXDC.BIN: sdlshim
	sh-elf-objcopy -S -R .stack -O binary $(TARGET) $@

1ST_READ.BIN: NXDC.BIN
	/opt/toolchains/dc/kos/utils/scramble/scramble $< $@

dist: 1ST_READ.BIN IP.BIN

distclean:
	rm -f 1ST_READ.BIN NXDC.BIN IP.BIN ip.txt nxdc.elf

clean:
	$(RM) $(OBJS) $(TARGET)

strip:
	sh-elf-strip $(TARGET) -o $(TARGET)

addr:
	sh-elf-addr2line -e $(TARGET) $(ARG)

run: $(TARGET)
	$(KOS_LOADER) -x $< -i DATA.ISO

data: dist
	cp 1ST_READ.BIN ../data/
	mkisofs -C 0,11702 -G IP.BIN -U -D -V "sdlshim" -p "histat" -o INPUT.ISO ../data

iso: data
	cdi4dc INPUT.ISO TEST.CDI
	$(RM) INPUT.ISO

emu:
	/mnt/c/opt/demul/demul.exe -run=dc -image=TEST.CDI

NXDCVER = 1.000

ip.txt: ../nx/ip.txt.in
	ver=V$(NXDCVER); \
	sed -e 's/[@]VERSION[@]/'"$$ver"/ -e 's/[@]DATE[@]/$(shell date '+%Y%m%d')/' < $< > $@

IP.BIN: ip.txt
	/opt/toolchains/dc/kos/utils/makeip/makeip ip.txt IP.BIN

# convert hey.png -colors 2 hey.pbm
icon_data_2bpp:
	echo 'static unsigned char icon_data_2bpp[] = {' >> $@.tmp
	hexdump -v -s 9 -e '16/1 "0x%02x, " "\n"' \
	in.pbm >> $@.tmp
	echo '		0 };' >> $@.tmp
