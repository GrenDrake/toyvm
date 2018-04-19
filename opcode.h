#ifndef OPCODE_H
#define OPCODE_H

enum opcode {
    op_exit,

    op_stkdup,

    op_pushb,
    op_pushs,
    op_pushw,
    op_readb,
    op_reads,
    op_readw,
    op_storeb,
    op_stores,
    op_storew,

    op_add,
    op_sub,
    op_mul,
    op_div,
    op_mod,

    op_saynum,
    op_saychar,
    op_saystr,

    op_jump,
    op_jumprel,
    op_jz,
    op_jnz,

    op_bad = -1
};

#endif
