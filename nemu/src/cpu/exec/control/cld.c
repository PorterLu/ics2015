#include "cpu/exec/helper.h"


int cld(swaddr_t eip){

    eflags.DF = 0;
    print_asm("cld");
    return 1;
}






