#include "cpu/exec/template-start.h"

#define instr cwt

static void do_execute()
{
    if(ops_decoded.is_operand_size_16 == 1)
    {
        if(MSB(cpu.eax & 0xff) == 1)
        {
            cpu.eax = cpu.eax | 0x0000ff00; 
        }
        else
        {
            cpu.eax = cpu.eax & 0xffff00ff;
        }
        print_asm("cwtw");
    }
    else
    {
        if(MSB(cpu.eax & 0x0000ffff) == 1)
        {
            cpu.eax = cpu.eax | 0xffff0000;
        }
        else
        {
            cpu.eax = cpu.eax | 0x0000ffff;
        }
        print_asm("cwtl");
    }
}


int concat3(instr,_,SUFFIX)(swaddr_t eip)
{
    do_execute();
    return 1;
}

#include "cpu/exec/template-end.h"