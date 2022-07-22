#include "cpu/exec/template-start.h"

#define instr movs
static void do_execute(){
    op_src->val = (DATA_TYPE)swaddr_read(cpu.esi,DATA_BYTE,DS);
    swaddr_write(cpu.edi,DATA_BYTE,op_src->val,ES);
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
}



int concat3(instr,_,SUFFIX)(swaddr_t eip){
    do_execute();
    return 1;
}


#include "cpu/exec/template-end.h"