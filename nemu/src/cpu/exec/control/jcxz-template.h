#include "cpu/exec/template-start.h"
#include "cpu/exec/helper.h"

#define instr jcxz
EFLAGS_state eflags;

static void do_execute(){
    uint32_t rel = (DATA_TYPE_S)op_src->val;
    rel = ops_decoded.is_operand_size_16?(uint16_t)rel:(uint32_t)rel; 
    if((ops_decoded.is_operand_size_16==0&&cpu.ecx == 0)||\
    (ops_decoded.is_operand_size_16==1&&(cpu.ecx&0x0000ffff)==0))
        cpu.eip += rel;
    print_asm_template1();

}

make_instr_helper(i)

#include "cpu/exec/template-end.h"