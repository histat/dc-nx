ronindir=../../libronin
ARMFLAGS=-mcpu=arm7 -ffreestanding  -O5 -funroll-loops
ARMFLAGS+= -I$(ronindir)/include -I.


arm_pxt_code.h: arm_pxt_code.bin
	./encode_armcode.pike < $< > $@

arm_pxt_code.bin: arm_pxt_code.elf
	arm-elf-objcopy -O binary $< $@

arm_pxt_code.elf: arm_pxt_code.o
	arm-elf-gcc $(ARMFLAGS) -Wl,-Ttext,0 -nostdlib -nostartfiles -o $@ $^ -lgcc -lgcc

arm_pxt_code.o: arm_pxt_code.c
	arm-elf-gcc -c -Wall $(ARMFLAGS) -o $@ $<

dump: arm_pxt_code.elf
	arm-elf-objdump -D $< > dump

clean:
	rm -f *.o *.bin *.elf
