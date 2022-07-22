#include "cpu/exec/helper.h"
#include "cpu/reg.h"

extern uint32_t page_translate(lnaddr_t eip);
int iret(swaddr_t eip)
{
    if(ops_decoded.is_operand_size_16 == 1)
    {
        cpu.eip = swaddr_read(cpu.esp,2,SS);
        cpu.cs.sreg = swaddr_read(cpu.esp+2,2,SS);
        eflags.val = swaddr_read(cpu.esp,2,SS);
        cpu.esp += 6;
        print_asm("iret");
    }
    else
    {
        cpu.eip = swaddr_read(cpu.esp,4,SS);
        //printf("%x\n",cpu.eip);
        //asm(".byte 0xd6");
        cpu.cs.sreg = (uint16_t) swaddr_read(cpu.esp + 4,4,SS);
        cpu.cs.gdesc = lnaddr_read(cpu.gdtr.base + cpu.cs.sreg,4);
        cpu.cs.gdesc |=(((uint64_t)lnaddr_read(cpu.gdtr.base + cpu.cs.sreg + 4 ,4))<<32);
        eflags.val = swaddr_read(cpu.esp+8,4,SS);
        cpu.esp += 12;
        //printf("%lx\n",cpu.cs.gdesc);
        print_asm("iret");
    }

    return 0;
}

