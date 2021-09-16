# myrv

## RISC-V experiments
Basically a simple RV32I emulator written from scratch (along with small test ESWs), based on the following documentation:
- [RV specs PDF](https://github.com/riscv/riscv-isa-manual/releases/download/Ratified-IMAFDQC/riscv-spec-20191213.pdf)
- [RV32I cheat sheet](https://metalcode.eu/2019-12-06-rv32i.html)
- [RV home](https://riscv.org/technical/specifications/)

![myrv screenshot](myrv.png)

## Prerequisites
In order to build native RISC-V esw, you must have a riscv toolchain available in your PATH:
- Clone https://github.com/riscv-software-src/riscv-gnu-toolchain
- Make sure autoconf, GNU awk, flex, bison and texinfo are installed, E.g.:
  - `$ sudo apt install gawk flex bison texinfo`
- `$ ./configure --prefix=$PWD/the_install --with-arch=rv32g --with-abi=ilp32d && make`

## Test it!
```
$ cd em
$ make clean test
rm -f *.o *.bin rv_test em esw elf
cc -Wall -Werror -g -O0 -o em em.c rv.c -lelf
riscv32-unknown-elf-gcc esw.c -g -O0 -fsigned-char -Ttext=0 -o esw -nostartfiles
./em esw
[Loaded ELF]
Hello world
[Memory state at finish : -5 (should be -5)]
```
