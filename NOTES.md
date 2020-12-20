EEI (Execution Environment Interface), eg:
-ABI (linux Application Binary Interface)
-SBI (Supervisor Binary Interface)

Implems :
- bare metal
- RV operationg systems
- RV hypervisor
- RV emulator

core : independent fetch unit
hart : Hardware Thread

XLEN : width of integer register

byte : 8 bits
word : 32 bits
halfword : 16 bits
doubleword : 64 bits
quadword : 128 bits

circular, single memory addres space=2^XLEN, no overflow, wraps around modulo 2^XLEN

different address range can :
1) be vacant (never accessible)
2) contain main memory (no side effects possible)
3) containe IO (can have side effects)

instruction execution => implicit memory read (instruction fetch)
load/store instruction => adds explicit memory access(es)

default memory consistency : Weak Memory Ordering
optionally : Total Store Ordering

default instructions fixed-length 32 bits, aligned on 32 bits
C extension : 16 bits instruction parcels, aligned on 16 bits, all instructions (16 and 32 bits) can
be aligned on 16 bits boundaries

IALIGN : alignment constraint (32 or 16 bits)
ILEN : maximum insn length supported, multiple of IALIGN


exception : unusual condition assoc with insn in current hart
interrupt : external async event
trap : transfer of control due to exception or interrupt

contained trap : ECALL user => supervisor
requested trap : syscall
invisible trap : transparently resumes; emulating missing insn, etc
fatal trap : terminates EE execution

RV32I : 40 unique insn
32 regs
x0 => reads 0; write ignored
x1 => link register by convention
x2 => stack pointer by convention
x5 => alternate link register by convention
Four core insn formats :
- R
- I
- S
- U
