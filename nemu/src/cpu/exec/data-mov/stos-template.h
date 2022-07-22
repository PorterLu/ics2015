#include "cpu/exec/template-start.h"

#define instr stos

static void do_execute()
{
    DATA_TYPE result = (DATA_TYPE)cpu.eax;
    uint32_t addr = cpu.edi;
    swaddr_write(addr,DATA_BYTE,result,ES);
    //printf("edi: %x\n",cpu.edi);
    if(eflags.DF == 0)
        cpu.edi += DATA_BYTE;
    else
        cpu.edi -= DATA_BYTE;
    print_asm(str(instr) str(SUFFIX));
}

int concat3(instr,_,SUFFIX)(swaddr_t eip)
{
    do_execute();
    return 1;
}


#include "cpu/exec/template-end.h"