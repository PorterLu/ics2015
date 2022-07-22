#include "cpu/exec/helper.h"


uint32_t ljmp(swaddr_t eip)
{
    uint32_t offset = instr_fetch(eip+1,4);
    uint16_t selector = instr_fetch(eip+5,2);
    cpu.sr[CS].sreg = selector ;
    //代码中最多支持一次读4字节
    cpu.sr[CS].gdesc = swaddr_read(cpu.gdtr.base + selector,4, DS);
    cpu.sr[CS].gdesc |= ((uint64_t)swaddr_read(cpu.gdtr.base + selector + 4,4,DS)<<32);
    cpu.eip = offset - 7;
    print_asm("ljmp 0x%x,0x%x",selector,offset);
    return 7;
}

