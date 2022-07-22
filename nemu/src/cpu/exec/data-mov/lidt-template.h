#include "cpu/exec/template-start.h"

#define instr lidt

static void do_execute()
{
    //printf("\n\n%x  %x\n\n",op_src->addr,op_src->val);
    uint16_t limit = lnaddr_read(op_src->addr,2);
    uint32_t base = lnaddr_read(op_src->addr+2,4);
    cpu.idtr.limit = limit;
    cpu.idtr.base = base;
    print_asm("lidt %s",op_src->str);
}

make_instr_helper(rm)

#include "cpu/exec/template-end.h"