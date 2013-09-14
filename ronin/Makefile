NETSERIAL = 1
NETCD = 0
LIBSD = 1

CCC = sh-elf-c++ -fno-rtti -fconserve-space
CC = sh-elf-gcc -Wall
LD = sh-elf-ld -EL
AS = sh-elf-as -little
AR = sh-elf-ar

#Must be O4 to handle long jumps correctly.
OPTIMISE=-O4 -ffreestanding -ffast-math -fschedule-insns2 -fomit-frame-pointer -fno-inline-functions -fno-defer-pop -fforce-addr -fstrict-aliasing -funroll-loops -fdelete-null-pointer-checks -fno-exceptions
CPUFLAGS = -ml  -m4-single-only
INCLUDES = -Iinclude -Iinclude/ronin


CCFLAGS = $(OPTIMISE) $(CPUFLAGS) -DDC -DDREAMCAST $(INCLUDES)

CFLAGS = $(CCFLAGS)

ARMFLAGS=-mcpu=arm7 -ffreestanding  -O5 -funroll-loops

DISTHEADERS=cdfs.h common.h dc_time.h gddrive.h maple.h matrix.h notlibc.h report.h ronin.h serial.h sincos_rroot.h soundcommon.h sound.h ta.h video.h vmsfs.h


# begin lwIP

LWCOREOBJS=lwip/core/mem.o lwip/core/memp.o lwip/core/netif.o \
	lwip/core/pbuf.o lwip/core/stats.o lwip/core/sys.o \
        lwip/core/tcp.o lwip/core/tcp_input.o \
        lwip/core/tcp_output.o lwip/core/udp.o 
LWCORE4OBJS=lwip/core/ipv4/icmp.o lwip/core/ipv4/ip.o \
	lwip/core/inet.o lwip/core/ipv4/ip_addr.o

LWAPIOBJS=lwip/api/api_lib.o lwip/api/api_msg.o lwip/api/tcpip.o \
	lwip/api/err.o lwip/api/sockets.o 

LWNETIFOBJS=lwip/netif/loopif.o \
	lwip/netif/tcpdump.o lwip/netif/arp.o

LWARCHOBJS=lwip/arch/dc/sys_arch.o lwip/arch/dc/thread_switch.o \
	lwip/arch/dc/netif/bbaif.o lwip/arch/dc/netif/rtk.o \
	lwip/arch/dc/netif/gapspci.o

LWIPOBJS=$(LWCOREOBJS) $(LWCORE4OBJS) $(LWAPIOBJS) $(LWNETIFOBJS) \
	$(LWARCHOBJS) lwip_util.o

# end lwIP

OBJECTS  = report.o ta.o maple.o video.o c_video.o vmsfs.o time.o display.o sound.o gddrive.o malloc.o matrix.o notlibc.o

ifeq "$(NETSERIAL)" "1"
OBJECTS += netserial.o $(LWIPOBJS)
else
OBJECTS += serial.o
endif

ifeq "$(NETCD)$(LIBSD)" "10"
OBJECTS += netcd.o
endif

ifeq "$(NETCD)$(LIBSD)" "00"
OBJECTS += cdfs.o
endif


all: include lib/crt0.o lib/libronin.a lib/libronin-sd.a lib/libronin-noserial.a lib/libronin-sd-noserial.a lib/libz.a


lib/crt0.o: crt0.s
	@test -d lib || mkdir lib
	$(AS) $< -o $@


ifeq "$(NETSERIAL)$(NETCD)$(LIBSD)" "110"
lib/libronin.a: $(OBJECTS) arm_sound_code.h
	@test -d lib || mkdir lib
	rm -f $@ && $(AR) rs $@ $(OBJECTS)

else
lib/libronin.a:
	@echo "disable NETCD"
endif

ifeq "$(NETSERIAL)$(NETCD)$(LIBSD)" "101"
lib/libronin-sd.a: $(OBJECTS) arm_sound_code.h
	$(MAKE) cleanish
	rm -f lib/libronin-sd.a
	$(MAKE) CFLAGS="$(CFLAGS) -D__SDCARD__" noserial-dummy
	rm -f $@ && $(AR) rs $@ $(OBJECTS) 

else
lib/libronin-sd.a:
	@echo "disable SD"
endif


noserial-dummy: $(OBJECTS) arm_sound_code.h
	@echo Dummy done.

ifeq "$(NETSERIAL)$(NETCD)$(LIBSD)" "001"
lib/libronin-sd-noserial.a:
	$(MAKE) cleanish
	rm -f lib/libronin-sd-noserial.a
	$(MAKE) CFLAGS="$(CFLAGS) -DNOSERIAL -D__SDCARD__" noserial-dummy
	rm -f $@ && $(AR) rs $@ $(OBJECTS)
else
lib/libronin-sd-noserial.a:
	@echo "enable NETSERIAL/NETCD"
endif

ifeq "$(NETSERIAL)$(NETCD)$(LIBSD)" "000"
lib/libronin-noserial.a:
	$(MAKE) cleanish
	rm -f lib/libronin-noserial.a
	$(MAKE) CFLAGS="$(CFLAGS) -DNOSERIAL" noserial-dummy
	rm -f $@ && $(AR) rs $@ $(OBJECTS)
else
lib/libronin-noserial.a:
	@echo "enable NETSERIAL/NETCD"
endif

lib/libz.a:
	@test -d lib || mkdir lib
	cd zlib; $(MAKE) libz.a
	@echo Making convenience links.
	rm -f lib/libz.a && ln -s ../zlib/libz.a lib/

include: $(DISTHEADERS) zlib/zlib.h zlib/zconf.h lwipopts.h lwip/include/lwip/*.h \
			lwip/include/netif/*.h lwip/include/ipv4/lwip/*.h \
			lwip/arch/dc/include/arch/*.h lwip/arch/dc/include/netif/*.h
	rm -rf include
	mkdir include
	mkdir include/ronin
	mkdir include/lwip
	mkdir include/netif
	mkdir include/arch
	for h in $(DISTHEADERS); do ln -s -f ../../$$h include/ronin/; done
	ln -s -f ../zlib/zlib.h ../zlib/zconf.h include/
	ln -s -f ../lwipopts.h include/
	cd include/lwip && ln -s -f ../../lwip/include/lwip/*.h ./
	cd include/lwip && ln -s -f ../../lwip/include/ipv4/lwip/*.h ./
	cd include/netif && ln -s -f ../../lwip/include/netif/*.h ./
	cd include/netif && ln -s -f ../../lwip/arch/dc/include/netif/*.h ./
	cd include/arch && ln -s -f ../../lwip/arch/dc/include/arch/*.h ./


#ARM sound code
arm_sound_code.h: arm_sound_code.bin
	./encode_armcode.pike < $< > $@

arm_sound_code.bin: arm_sound_code.elf
	arm-elf-objcopy -O binary $< $@

arm_sound_code.elf: arm_startup.o arm_sound_code.o
	arm-elf-gcc $(ARMFLAGS) -Wl,-Ttext,0 -nostdlib -nostartfiles -o $@ $^ -lgcc -lgcc

arm_sound_code.o: arm_sound_code.c soundcommon.h
	arm-elf-gcc -c -Wall $(ARMFLAGS) -o $@ $<

arm_startup.o: arm_startup.s
	arm-elf-as -mcpu=arm7 -o $@ $<

sound.o: arm_sound_code.h

cleanish:
	rm -f $(OBJECTS) \
	      arm_sound_code.h arm_sound_code.bin arm_sound_code.elf \
	      arm_sound_code.o arm_startup.o

clean: cleanish
	rm -f lib/crt0.o
	rm -f lib/libronin.a
	rm -f lib/libronin-sd.a
	rm -f lib/libronin-noserial.a
	rm -f lib/libronin-sd-noserial.a
	rm -f lib/libz.a
	cd zlib && $(MAKE) clean


