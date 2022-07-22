#include "cpu/exec/helper.h"

int cwd_w(swaddr_t eip){
    short data = cpu.eax;
    if(data<0)
        cpu.edx |= 0xffff;
    else
        cpu.edx &= 0xffff0000;
    print_asm("cwdw");
    return 1;
}

int cwd_l(swaddr_t eip){
    int data = cpu.eax;
    if(data<0)
        cpu.edx = 0xffffffff;
    else
        cpu.edx = 0;
    print_asm("cwdl");
    return 1;
}


make_helper_v(cwd)


