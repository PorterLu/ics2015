#include "cpu/exec/template-start.h"

#define instr leave

static void do_execute(){

    cpu.esp = cpu.ebp;
    DATA_TYPE result = swaddr_read(cpu.esp,DATA_BYTE, SS);
    cpu.ebp = result;
    cpu.esp += DATA_BYTE;

    print_asm(str(instr) str(SUFFIX));
}


int concat3(instr , _ , SUFFIX)(swaddr_t eip)
{
    do_execute();
    return 1;
}

#include "cpu/exec/template-end.h"