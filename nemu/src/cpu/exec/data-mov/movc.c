#include "cpu/exec/helper.h"
#include "trap.h"
#define instr mov
#define SUFFIX l

uint32_t mov_c2r(swaddr_t eip)
{
	uint8_t modR_M = swaddr_read(eip+1,1,CS);
	op_src->reg = modR_M & 0x07;
	op_src->type = OP_TYPE_REG;
	switch((modR_M & 0x38)>>3)
	{ 
		case 0:
			write_operand_l(op_src,cpu.cr0.val);
			print_asm(str(instr) str(SUFFIX) " cr0,%%%s",	regsl[op_src->reg]);
			break;
		case 3:
			write_operand_l(op_src,cpu.cr3.val);
			print_asm(str(instr) str(SUFFIX) " cr3,%%%s",regsl[op_src->reg]);
	}
	return 2;
}


uint32_t mov_r2c(swaddr_t eip)
{

	//if(cpu.eip == 0x1014db)
	//	asm(".byte 0xd6");
	uint8_t modR_M = swaddr_read(eip+1,1,CS);
	op_src->reg = modR_M & 0x07;
	//asm(".byte 0xd6");
	switch((modR_M & 0x38)>>3)
	{
		case 0:
			cpu.cr0.val = cpu.gpr[op_src->reg]._32;
			print_asm(str(instr) str(SUFFIX) " %%%s,cr0",regsl[op_src->reg]);
			break;
		case 3:
			cpu.cr3.val = cpu.gpr[op_src->reg]._32;
			print_asm(str(instr) str(SUFFIX) " %%%s,cr3",regsl[op_src->reg]);
	}
	return 2;
}


