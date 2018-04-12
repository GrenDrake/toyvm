#ifndef OPCODE_H
#define OPCODE_H

enum opcode {
    op_exit,
    op_loadi,
    op_add,
    op_saynum,
    op_jump,
    op_jz,
    op_jnz,
    op_sub,
    op_jumprel,
    op_mul,
    op_div,
    op_mod,
    op_loadwi,
    op_loadwr,
    
    op_bad = -1
};

#endif
