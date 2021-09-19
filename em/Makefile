TARGET:=rv_test
TARGET+=em
TARGET+=esw
TARGET+=em_esw
#TARGET+=em_esw.bin

CFLAGS:=-Wall -Werror
CFLAGS+=-g -O0

all: $(TARGET)

HAVE_ELF:=1
ifdef HAVE_ELF
em:LDLIBS+=-lelf
endif

elf:LDLIBS+=-lelf

ifdef V
# VERBOSE:=-v -v
VERBOSE:=-v
endif
# test: em esw
# 	./em esw $(VERBOSE)

test: em em_esw
	./em em_esw $(VERBOSE)

check: rv_test
	./rv_test

rv_test: rv_test.c rv.c
	$(CC) $(CFLAGS) -o $@ $^

em: em.c rv.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

esw: esw.c
#	riscv32-unknown-elf-gcc $< -g -O0 -fsigned-char -Ttext=0 -o esw -nostartfiles
	riscv32-unknown-elf-gcc $< -g -O0 -fsigned-char -Ttext=0 -o $@

esw.bin: esw
	riscv32-unknown-elf-objcopy -Obinary esw $@

em_esw: em_esw.o
	riscv32-unknown-elf-gcc $< -o $@ -nostartfiles

%.o: %.s
	riscv32-unknown-elf-as -o $@ $<

%.bin: %.o
	riscv32-unknown-elf-objcopy -Obinary $< $@

clean:
	$(RM) *.o *.bin rv_test em em_esw esw elf qcheck
