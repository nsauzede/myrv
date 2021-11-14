#!/bin/sh

riscv32-unknown-elf-gdb -q -nx -ex 'set debug remote on' -ex 'target remote 127.0.0.1:1235' esw
