#include "cpu/exec/template-start.h"

#define instr bt

static void do_execute()
{
    uint32_t base = op_dest->val;
    uint32_t offset = op_src->val;
    if(ops_decoded.opcode == 0xba)
        offset = (uint8_t)offset;

    eflags.CF = ((0x01<<offset)&base)>>offset;
    print_asm_template2();

}

make_instr_helper(si2rm)
make_instr_helper(r2rm)

#include "cpu/exec/template-end.h"
