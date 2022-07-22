#include "cpu/exec/helper.h"
#include "cpu/decode/modrm.h"
#define instr movsx
#define reg_l(index) (cpu.gpr[check_reg_index(index)]._32)
#define reg_w(index) (cpu.gpr[check_reg_index(index)]._16)
#define reg_b(index) (cpu.gpr[check_reg_index(index) & 0x3]._8[index >> 2])

int movsx_rm2r_w(swaddr_t eip)
{
    op_src->size = 1;
    int len = read_ModR_M(eip+1,op_src,op_dest);
    //reg_w(op_dest->reg) = (uint16_t)(int8_t)op_src->val;
    write_operand_w(op_dest,(uint16_t)(int8_t)op_src->val);
#ifdef DEBUG
	snprintf(op_dest->str, OP_STR_SIZE, "%%%s", regsl[op_dest->reg]);
#endif
    print_asm(str(instr) "w" " %s,%s", op_src->str, op_dest->str);
    return len + 1;
}

int movsx_rm2r_l(swaddr_t eip)
{
    op_src->size = 1;
    int len = read_ModR_M(eip+1,op_src,op_dest);
    //reg_l(op_dest->reg) = (uint32_t)(int8_t)op_src->val;
    write_operand_l(op_dest,(uint32_t)(int8_t)op_src->val);
#ifdef DEBUG
	snprintf(op_dest->str, OP_STR_SIZE, "%%%s", regsl[op_dest->reg]);
#endif
    print_asm(str(instr) "l" " %s,%s", op_src->str, op_dest->str);
    return len + 1;
}

int movsx_rm2r(swaddr_t eip)
{
    op_src->size = 2;
    int len = read_ModR_M(eip+1,op_src,op_dest);
    //reg_l(op_dest->reg) = (uint32_t)(int16_t)op_src->val;
    write_operand_l(op_dest,(uint32_t)(int16_t)op_src->val);
#ifdef DEBUG
	snprintf(op_dest->str, OP_STR_SIZE, "%%%s", regsl[op_dest->reg]);
#endif
    print_asm(str(instr) "wl" " %s,%s", op_src->str, op_dest->str);
    return len + 1;
}



make_helper_v(movsx_rm2r)

