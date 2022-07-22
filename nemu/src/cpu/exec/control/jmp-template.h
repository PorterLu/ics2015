#include "cpu/exec/template-start.h"

#define instr jmp

static void do_execute(){
    if(ops_decoded.opcode==0xff)
    {
        int len = concat(decode_rm_,SUFFIX)(cpu.eip+1)+1;
        cpu.eip = (uint32_t)(DATA_TYPE_S)op_src->val - len ;
    }
    else
        cpu.eip = cpu.eip + (uint32_t)(DATA_TYPE_S)op_src->val;
    print_asm_template1();
}

int concat3(instr,_,SUFFIX)(swaddr_t eip){
   return idex(eip,concat3(decode_i,_,SUFFIX),do_execute);;
}

make_instr_helper(rm)


#include "cpu/exec/template-end.h"