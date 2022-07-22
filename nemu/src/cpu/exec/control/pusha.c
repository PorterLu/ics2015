#include "cpu/exec/helper.h"
#include "cpu/reg.h"

int pusha(swaddr_t eip)
{
    if(ops_decoded.is_operand_size_16 == 1)
    {
        uint16_t temp =(uint16_t) cpu.esp;
        swaddr_write(cpu.esp-2,2,(uint16_t)cpu.eax,SS);
        swaddr_write(cpu.esp-4,2,(uint16_t)cpu.ecx,SS);
        swaddr_write(cpu.esp-6,2,(uint16_t)cpu.edx,SS);
        swaddr_write(cpu.esp-8,2,(uint16_t)cpu.ebx,SS);
        swaddr_write(cpu.esp-10,2,temp,SS);
        swaddr_write(cpu.esp-12,2,(uint16_t)cpu.ebp,SS);
        swaddr_write(cpu.esp-14,2,(uint16_t)cpu.esi,SS);
        swaddr_write(cpu.esp-16,2,(uint16_t)cpu.edi,SS);
        cpu.esp -= 16;
        print_asm("pushaw");
        return 1;
    }
    else
    {
        uint32_t temp = cpu.esp;
        swaddr_write(cpu.esp-4,4,cpu.eax,SS);
        swaddr_write(cpu.esp-8,4,cpu.ecx,SS);
        swaddr_write(cpu.esp-12,4,cpu.edx,SS);
        swaddr_write(cpu.esp-16,4,cpu.ebx,SS);
        swaddr_write(cpu.esp-20,4,temp,SS);
        swaddr_write(cpu.esp-24,4,cpu.ebp,SS);
        swaddr_write(cpu.esp-28,4,cpu.esi,SS);
        swaddr_write(cpu.esp-32,4,cpu.edi,SS);
        cpu.esp -= 32;
        print_asm("pushal");
        return 1;
    }
} 