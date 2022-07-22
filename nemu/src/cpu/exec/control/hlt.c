#include "cpu/exec/helper.h"
extern void check_device_update();
int hlt(swaddr_t eip)
{
    while(cpu.INTR == 0)
    {
        check_device_update();
    }
    print_asm("hlt");
    return 1;
}