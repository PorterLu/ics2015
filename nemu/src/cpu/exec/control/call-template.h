#include "cpu/exec/template-start.h"
#include "cpu/decode/modrm.h"

#define instr call

static void do_execute(){
    if(ops_decoded.opcode == 0xff)
    {
        int len = read_ModR_M(cpu.eip+1,op_src,op_dest);
        swaddr_write(cpu.esp - DATA_BYTE, DATA_BYTE, cpu.eip + len+1,SS);
        
        cpu.eip = (DATA_TYPE)op_src->val - len - 1;
        cpu.esp -= DATA_BYTE;
        print_asm_template1();
        return ;
    }
    
    int target = (DATA_TYPE)cpu.eip + (uint32_t)(DATA_TYPE_S)op_src->val ;
    int len  = DATA_BYTE + 1;
    swaddr_write(cpu.esp - DATA_BYTE,DATA_BYTE, cpu.eip + len,SS);
    cpu.esp -= DATA_BYTE;
    cpu.eip = target ;//exec后在cpu_exec()中会加上指令长
    print_asm_template1();
}

#if DATA_BYTE == 2 || DATA_BYTE ==4
make_instr_helper(i)
make_instr_helper(rm)
#endif


#include "cpu/exec/template-end.h"