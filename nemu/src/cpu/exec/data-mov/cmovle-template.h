#include "cpu/exec/template-start.h"

#define instr cmovle


static void do_execute(){
    if( eflags.SF != eflags.OF || eflags.ZF == 1 )
        OPERAND_W(op_dest, op_src->val);
	print_asm_template2();
}


make_instr_helper(rm2r)

#include "cpu/exec/template-end.h"