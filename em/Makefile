TARGET:=rv_test
TARGET+=em
TARGET+=em_esw.bin

_SYS := $(shell uname 2>/dev/null || echo Unknown)
_SYS := $(patsubst MSYS%,MSYS,$(_SYS))
_SYS := $(patsubst MINGW%,MinGW,$(_SYS))
ifneq ($(filter $(_SYS),MSYS MinGW),)
WIN32 := 1
endif

ifndef WIN32
HAVE_ELF:=1
HAVE_GDBSTUB:=1
HAVE_QCHECK:=1
endif

ifdef HAVE_ELF
TARGET+=esw
TARGET+=em_esw
endif

CFLAGS:=-Wall -Werror
CFLAGS+=-g -O0
CFLAGS+=-pthread

ifdef HAVE_ELF
CFLAGS+=-DHAVE_ELF
endif

ifdef HAVE_GDBSTUB
CFLAGS+=-DHAVE_GDBSTUB
endif

ifdef HAVE_QCHECK
CFLAGS+=-DHAVE_QCHECK
endif

all: $(TARGET)

EM_SRC:=em.c em_sys.c rv.c

ifdef HAVE_ELF
em:LDLIBS+=-lelf
elf:LDLIBS+=-lelf
EM_SRC+=em_elf.c
endif

ifdef HAVE_GDBSTUB
EM_SRC+=em_gdbstub.c
endif

ifdef HAVE_QCHECK
EM_SRC+=em_qcheck.c
endif

ifdef V
# VERBOSE:=-v -v
VERBOSE:=-v
endif
# test: em esw
# 	./em esw $(VERBOSE)

test: em
	./em $(VERBOSE)

check: rv_test
	./rv_test

rv_test: rv_test.c rv.c
	$(CC) $(CFLAGS) -o $@ $^

em: $(EM_SRC)
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
