#include "cpu/exec/helper.h"
#include "device/port-io.h"
#include "cpu/reg.h"

int out_b(swaddr_t eip)
{
    pio_write((uint16_t)cpu.edx,1,(uint32_t)(uint8_t)cpu.eax);
    print_asm("out %%al,(%%dx)");
    return 1;
}

int out_v(swaddr_t eip)
{
    if(ops_decoded.is_operand_size_16 == 1)
    {
        pio_write((uint16_t)cpu.edx,2,(uint16_t)cpu.eax);
        print_asm("out %%ax,(%%dx)");
    }
    else
    {
        pio_write((uint16_t)cpu.edx,4,cpu.eax);
        print_asm("out %%eax,(%%dx)");
    }
    return 1;
}