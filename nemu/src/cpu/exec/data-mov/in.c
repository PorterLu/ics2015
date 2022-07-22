#include "cpu/exec/helper.h"
#include "device/port-io.h"
#include "cpu/reg.h"

#define instr in
int in_b(swaddr_t eip)
{
    cpu.eax = (uint8_t) pio_read((uint16_t)cpu.edx,1);
    print_asm("inb (%%dx),%%al");
    return 1;
}


int in_v(swaddr_t eip)
{
    if(ops_decoded.is_operand_size_16 == 1)
    {
        cpu.eax = (uint16_t)pio_read((uint16_t)cpu.edx,2);
        print_asm("in (%%dx),%%ax");
        
    }
    else
    {   
        cpu.eax = pio_read((uint16_t)cpu.edx,4);
        print_asm("in (%%dx),%%eax");
    }
    return 1;
}


