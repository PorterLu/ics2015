#include "cpu/exec/template-start.h"

#define instr push

static void do_execute(){
#if DATA_BYTE == 1
    if(ops_decoded.is_operand_size_16)
    {
        cpu.esp -= 2;
        uint16_t data = (uint16_t)(int8_t)op_src->val;
        swaddr_write(cpu.esp,2,data,SS);
        print_asm_template1();
        return ;
    }
    else
    {
        cpu.esp -= 4;
        uint32_t data = (uint32_t)(int8_t)op_src->val;
        swaddr_write(cpu.esp,4,data,SS);
        print_asm_template1();
        return;
    }

#endif
    cpu.esp -= DATA_BYTE;
    DATA_TYPE data = op_src->val;
    swaddr_write(cpu.esp,DATA_BYTE,data,SS);
    print_asm_template1();
}


make_instr_helper(i)
#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(r)
make_instr_helper(rm)
#endif

#include "cpu/exec/template-end.h"