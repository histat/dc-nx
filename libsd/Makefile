RONINDIR = ../libronin

CC = sh-elf-gcc -ml -m4-single-only
AR = sh-elf-ar

NAME = sdcard-test
TARGET = $(NAME).elf
STRIPELF = $(NAME).strip.elf

LIBS = -L$(RONINDIR)/lib -lronin -lz -lm

LIBS =-Wl,--start-group -L$(RONINDIR)/lib -lronin -lc -lgcc -Wl,--end-group -lz -lm
ASFLAGS =
CFLAGS = -Wall -O4 -fomit-frame-pointer -I$(RONINDIR)/include -I.
LDFLAGS = -Wl,--oformat,elf32-shl,-T$(RONINDIR)/tools/shlelf.xc -nostartfiles -nostdlib $(RONINDIR)/lib/crt0.o

OBJS = sci.o crc.o ccsbcs.o ff.o sd.o

all: libsd.a

$(TARGET): $(OBJS) main.o
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS) main.o $(LIBS)
	sh-elf-strip -o $(STRIPELF) $@

libsd.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

clean:
	rm -f $(OBJS) $(TARGET) $(STRIPELF)

run:
	ipupload.pike < $(TARGET)
