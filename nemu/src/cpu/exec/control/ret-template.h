#include "cpu/exec/template-start.h"
#define instr ret

static void do_execute(){
    DATA_TYPE result = swaddr_read(cpu.esp,DATA_BYTE, SS);
    cpu.eip = (uint32_t)result;
    cpu.esp += DATA_BYTE;
    print_asm(str(instr) str(SUFFIX)); 
}


int concat3(instr,_,SUFFIX)(swaddr_t eip){
    do_execute();
    return 0;
}

int concat3(instr,_i_,SUFFIX)(swaddr_t eip){
    
    DATA_TYPE result = swaddr_read(cpu.esp,DATA_BYTE,SS);
    uint16_t imm = instr_fetch(eip+1,2);
    cpu.eip = (uint32_t)result;
    cpu.esp = cpu.esp + imm + DATA_BYTE;
    print_asm(str(instr) str(SUFFIX) " %x ",imm); 

    return 0;
}




#include "cpu/exec/template-end.h"