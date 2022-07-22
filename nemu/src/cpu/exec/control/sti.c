#include "cpu/exec/helper.h"
#include "cpu/reg.h"

int sti(swaddr_t eip)
{
    eflags.IF = 1;
    print_asm("sti");
    return 1;
}