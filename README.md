# myrv_

RISC-V experiments

# Prerequisites
In order to build native RISC-V esw, you must have a riscv toolchain available in your PATH:
- Clone https://github.com/riscv-software-src/riscv-gnu-toolchain
- Make sure GNU awk, flex, bison and texinfo are installed, E.g.:
  - `sudo apt install gawk flex bison texinfo`
- `./configure --prefix=$PWD/the_install --with-arch=rv32g --with-abi=ilp32d && make`

