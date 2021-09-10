TARGET:=em
#TARGET+=esw.bin

all: $(TARGET)

test: rv_test
	./rv_test

rv_test: rv_test.c rv.c
	$(CC) -o $@ $^

em: em.c rv.c
	$(CC) -o $@ $^

esw.o: esw.c
	../../riscv-gnu-toolchain/the_install/bin/riscv32-unknown-elf-gcc -c $< -g -O2 -fsigned-char
	../../riscv-gnu-toolchain/the_install/bin/riscv32-unknown-elf-objdump -d -S $@

esw.bin: esw.o
	../../riscv-gnu-toolchain/the_install/bin/riscv32-unknown-elf-objcopy -Obinary $< $@
	od -tx1 $@

clean:
	$(RM) *.o *.bin rv_test em
