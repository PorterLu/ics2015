#include "cpu/exec/helper.h"
#include "cpu/reg.h"
#include "pop.h"

int popa(swaddr_t eip)
{
    if(ops_decoded.is_operand_size_16 == 1)
    {
        cpu.edi = swaddr_read(cpu.esp,2,SS);
        cpu.esi = swaddr_read(cpu.esp+2,2,SS);
        cpu.ebp = swaddr_read(cpu.esp+4,2,SS);
        cpu.ebx = swaddr_read(cpu.esp+8,2,SS);
        cpu.edx = swaddr_read(cpu.esp+10,2,SS);
        cpu.ecx = swaddr_read(cpu.esp+12,2,SS);
        cpu.eax = swaddr_read(cpu.esp+14,2,SS);
        cpu.esp += 16;
        print_asm("popaw");
        return 1 ;
    }
    else
    {
        cpu.edi = swaddr_read(cpu.esp,4,SS);
        cpu.esi = swaddr_read(cpu.esp+4,4,SS);
        cpu.ebp = swaddr_read(cpu.esp+8,4,SS);
        cpu.ebx = swaddr_read(cpu.esp+16,4,SS);
        cpu.edx = swaddr_read(cpu.esp+20,4,SS);
        cpu.ecx = swaddr_read(cpu.esp+24,4,SS);
        cpu.eax = swaddr_read(cpu.esp+28,4,SS);
        cpu.esp += 32;
        print_asm("popal");
        return 1;
    }
}