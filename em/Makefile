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

CFLAGS:=-Wall -Werror -Wextra
#CFLAGS+=-Wconversion -Wsign-conversion
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
TARGET+=qcheck
TARGET+=qcheck2
endif

all: $(TARGET)

EM_SRC:=em.c em_sys.c rv.c

ifdef HAVE_ELF
em:LDLIBS+=-lelf
elf:LDLIBS+=-lelf
EM_SRC+=em_elf.c
endif

ifdef HAVE_QCHECK
#EM_SRC+=em_qcheck.c
EM_SRC+=em_qcheck2.c
endif

ifdef V
# VERBOSE:=-v -v
VERBOSE:=-v
endif

EM_OBJ:=$(patsubst %.c, %.o, $(EM_SRC))

# test: em esw
# 	./em esw $(VERBOSE)

test: em
	./em $(VERBOSE)

check: rv_test
	./rv_test

librspd/librspd.h:
	git submodule update --init --recursive

rv.o: CFLAGS+=-Ilibrspd
rv.o: rv.c librspd/librspd.h

rv_test: rv_test.o rv.o
	$(CC) $(CFLAGS) -o $@ $^

em: $(EM_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

#RISCV_PREFIX:=riscv32-unknown-elf-
RISCV_PREFIX:=riscv64-unknown-elf-
#RV_FLAGS:=-march=rv32g -mabi=ilp32d
#RV_FLAGS:=-march=rv64g -mabi=lp64d
RV_FLAGS:=-march=rv32im -mabi=ilp32
esw: esw.c
#	$(RISCV_PREFIX)gcc $< -g -O0 -fsigned-char -Ttext=0 -o esw -nostartfiles
	$(RISCV_PREFIX)gcc $< -g -O0 -fsigned-char -Ttext=0 -o $@ $(RV_FLAGS)

esw.bin: esw
	$(RISCV_PREFIX)objcopy -Obinary esw $@

em_esw: em_esw.o
	$(RISCV_PREFIX)gcc $< -o $@ -nostartfiles $(RV_FLAGS)

%.o: %.s
	$(RISCV_PREFIX)as -o $@ $< $(RV_FLAGS)

%.bin: %.o
	$(RISCV_PREFIX)objcopy -Obinary $< $@

clean:
	$(RM) *.o *.bin rv_test em em_esw esw elf qcheck qcheck2

clobber: clean

mrproper: clobber
