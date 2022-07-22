#include "cpu/exec/template-start.h"

#define instr jl
EFLAGS_state eflags;

static void do_execute(){
    uint32_t rel = (DATA_TYPE_S)op_src->val;
    rel = ops_decoded.is_operand_size_16?(uint16_t)rel:(uint32_t)rel; 
    if(eflags.SF != eflags.OF)
        cpu.eip += rel;
    print_asm_template1();

}

make_instr_helper(i)

#include "cpu/exec/template-end.h"