#include "cpu/exec/helper.h"
#include "cpu/reg.h"

int cli(swaddr_t eip)
{
    eflags.IF = 0;
    print_asm("cli");
    return 1;
}