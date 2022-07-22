#include "cpu/exec/template-start.h"

#define instr cmp
EFLAGS_state eflags;

static void do_execute (){
    eflags.CF=0,eflags.OF=0,eflags.SF=0,eflags.ZF=0,eflags.PF=0;
    DATA_TYPE result = (DATA_TYPE)op_dest->val - op_src->val;
    if(op_dest->val < op_src->val) eflags.CF = 1;
    if(op_dest->val == op_src->val) eflags.ZF = 1;
    if((MSB(result) == 1)) eflags.SF = 1;
    eflags.OF = (MSB(op_dest->val)^MSB(op_src->val))&&(MSB(op_dest->val)^(MSB(result)));
    uint32_t count=0,i=0,tmp=0x01;
    for(i=0;i<8;i++)
    {
        if((tmp&result) == 1)
            count++;
        tmp = tmp << 1;
    }
    if(count%2 == 0) eflags.PF = 1;
    print_asm_template2();
}


make_instr_helper(i2a)
make_instr_helper(i2rm)
make_instr_helper(r2rm)
make_instr_helper(rm2r)
#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(si2rm)
#endif

#include "cpu/exec/template-end.h"