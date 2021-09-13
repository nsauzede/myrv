TARGET:=rv_test
#TARGET+=em
#TARGET+=em_esw.bin

all: $(TARGET)

test: rv_test
	./rv_test

rv_test: rv_test.c rv.c
	$(CC) -o $@ $^

em: em.c rv.c | em_esw.bin
	$(CC) -o $@ $^

esw.o: esw.c
	../../riscv-gnu-toolchain/the_install/bin/riscv32-unknown-elf-gcc -c $< -g -O2 -fsigned-char
	../../riscv-gnu-toolchain/the_install/bin/riscv32-unknown-elf-objdump -d -S $@

%.o: %.s
	../../riscv-gnu-toolchain/the_install/bin/riscv32-unknown-elf-as -o $@ $<
	../../riscv-gnu-toolchain/the_install/bin/riscv32-unknown-elf-objdump -d -S $@

%.bin: %.o
	../../riscv-gnu-toolchain/the_install/bin/riscv32-unknown-elf-objcopy -Obinary $< $@
	od -tx1 $@

clean:
	$(RM) *.o *.bin rv_test em
