#include "cpu/exec/template-start.h"

#define instr lgdt


static void do_execute()
{
    //gdtr寄存器的内容为48位，前16位是界限，后32位是base
    //这里没有进行转换，因为cr0的保护模式位还没有置位
    //printf("-1\n");
    uint16_t limit  = lnaddr_read(op_src->addr,2);
    //printf("0\n");
    uint32_t base = lnaddr_read(op_src->addr+2,4);
    //printf("1\n");
    cpu.gdtr.limit = limit;
    cpu.gdtr.base = base;

    print_asm("lgdt %s",op_src->str);
}

make_instr_helper(rm)

#include "cpu/exec/template-end.h"