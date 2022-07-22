#include "cpu/exec/template-start.h"

#define instr xor

static void do_execute () {
	DATA_TYPE result = op_dest->val ^ op_src->val;
	OPERAND_W(op_dest, result);

	/* TODO: Update EFLAGS. */
	eflags.CF = 0;
    eflags.OF = 0;
    eflags.SF = MSB(result);
    if(result == 0) eflags.ZF = 1;
    else eflags.ZF = 0;
    uint32_t count=0,i=0,tmp=0x01;
    for(i=0;i<8;i++)
    {
        if((tmp&result) == 1)
            count++;
        tmp = tmp << 1;
    }
    if(count%2 == 0) eflags.PF = 1;
    else eflags.PF = 0;

	print_asm_template2();
}

make_instr_helper(i2a)
make_instr_helper(i2rm)
#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(si2rm)
#endif
make_instr_helper(r2rm)
make_instr_helper(rm2r)

#include "cpu/exec/template-end.h"
