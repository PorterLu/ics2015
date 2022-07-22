#include "cpu/exec/template-start.h"
#include "cpu/reg.h"

#define instr cmps

int concat3(instr,_,SUFFIX)(swaddr_t eip){
    op_dest->val = swaddr_read(cpu.edi,DATA_BYTE, ES);
    op_src->val = swaddr_read(cpu.esi,DATA_BYTE, DS);
    uint32_t result = op_dest->val - op_src->val;
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
    print_asm("cmps" str(SUFFIX));

    if(eflags.DF == 0)
    {
        cpu.edi += DATA_BYTE;
        cpu.esi += DATA_BYTE;
    }
    else
    {
        cpu.edi -= DATA_BYTE;
        cpu.esi -= DATA_BYTE;
    }

    print_asm(str(instr) str(SUFFIX));
    return 1;
}

#include "cpu/exec/template-end.h"
