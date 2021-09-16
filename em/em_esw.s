.section .text
.global _start

_start:
        lui a0, %hi(DATA_BASE)
        addi a0, a0, %lo(DATA_BASE)
        lw t0, 0(a0)
        lw t1, 4(a0)
        add t2, t0, t1
        sw t2, 8(a0)

        lw t0, 0(a0)
        lw t1, 4(a0)
        add t2, t0, t1
        sub t3, t0, t1
        mul t4, t0, t1
        div t5, t0, t1
        rem t6, t0, t1
        sw t2, 8(a0)
        sw t3, 12(a0)
        sw t4, 16(a0)
        sw t5, 20(a0)
        lw t0, 0(a0)
        lw t1, 4(a0)
        add t2, t0, t1
        sub t3, t0, t1
        mul t4, t0, t1
        div t5, t0, t1
        sw t2, 8(a0)
        sw t3, 12(a0)
        sw t4, 16(a0)
        sw t5, 20(a0)
        lw t0, 0(a0)
        lw t1, 4(a0)
        add t2, t0, t1
        sub t3, t0, t1
        mul t4, t0, t1
        div t5, t0, t1
        sw t2, 8(a0)
        sw t3, 12(a0)
        sw t4, 16(a0)
        sw t5, 20(a0)
        lw t0, 0(a0)
        lw t1, 4(a0)
        add t2, t0, t1
        sub t3, t0, t1
/*        mul t4, t0, t1
        div t5, t0, t1*/
        sw t2, 8(a0)
        sw t3, 12(a0)
        sw t4, 16(a0)
        sw t5, 20(a0)

	ebreak

.equ DATA_BASE, 0x2000
