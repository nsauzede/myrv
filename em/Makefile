TARGET:=rv_test
#TARGET+=em
#TARGET+=em_esw.bin

CFLAGS:=-Wall -Werror
CFLAGS+=-g -O0

all: $(TARGET)

test: rv_test
	./rv_test

rv_test: rv_test.c rv.c
	$(CC) $(CFLAGS) -o $@ $^

em: em.c rv.c | em_esw.bin
	$(CC) $(CFLAGS) -o $@ $^

esw.bin: esw.c
#	../../riscv-gnu-toolchain/the_install/bin/riscv32-unknown-elf-gcc $< -g -O0 -fsigned-char -nostdlib -Ttext=0 -o esw
	../../riscv-gnu-toolchain/the_install/bin/riscv32-unknown-elf-gcc $< -g -O0 -fsigned-char -Ttext=0 -o esw -nostartfiles
#	../../riscv-gnu-toolchain/the_install/bin/riscv32-unknown-elf-objdump -d -S esw
	../../riscv-gnu-toolchain/the_install/bin/riscv32-unknown-elf-objcopy -Obinary esw $@
#	od -tx1 $@

%.o: %.s
	../../riscv-gnu-toolchain/the_install/bin/riscv32-unknown-elf-as -o $@ $<
#	../../riscv-gnu-toolchain/the_install/bin/riscv32-unknown-elf-objdump -d -S $@

%.bin: %.o
	../../riscv-gnu-toolchain/the_install/bin/riscv32-unknown-elf-objcopy -Obinary $< $@
#	od -tx1 $@

clean:
	$(RM) *.o *.bin rv_test em esw
