#include "cpu/exec/template-start.h"

#define instr mov

static void do_execute() {
	OPERAND_W(op_dest, op_src->val);
	print_asm_template2();
}

make_instr_helper(i2r)
make_instr_helper(i2rm)
make_instr_helper(r2rm)
make_instr_helper(rm2r)

/*

	take "i2r" as example,extend the macro, we get the function:
	int mov_i2r_b(swaddr_t eip){
		return idex(eip,decode_i2r_b,do_excute);
	}
*/
#if DATA_BYTE == 2
make_helper(mov_s2rm_w)
{
	int len = decode_rm_w(eip+1);
	uint8_t modR_M = instr_fetch(eip+1,1);
	uint8_t index = (modR_M & 0x38)>>3;
	OPERAND_W(op_src,cpu.sr[index].sreg); 
	switch(index)
	{
		case 0: print_asm("movw %%es,%s",op_src->str);break;
		case 1: print_asm("movw %%cs,%s",op_src->str);break;
		case 2: print_asm("movw %%ss,%s",op_src->str);break;
		case 3: print_asm("movw %%ds,%s",op_src->str);break;
		case 4: print_asm("movw %%fs,%s",op_src->str);break;
		case 5: print_asm("movw %%gs,%s",op_src->str);
	}
	return len + 1;
}

make_helper(mov_rm2s_w)
{
	int len = decode_rm_w(eip+1);
	uint8_t modR_M = instr_fetch(eip+1,1);
	uint8_t index = (modR_M & 0x38)>>3;
	cpu.sr[index].sreg = op_src->val;
	cpu.sr[index].gdesc = (uint64_t)lnaddr_read(cpu.gdtr.base + cpu.sr[index].sreg ,4)&0x00000000ffffffff;
	cpu.sr[index].gdesc |=((uint64_t)lnaddr_read(cpu.gdtr.base + cpu.sr[index].sreg  + 4,4))<<32;
	switch(index)
	{
		case 0: print_asm("movw %s,%%es",op_src->str);break;
		case 1: print_asm("movw %s,%%cs",op_src->str);break;
		case 2: print_asm("movw %s,%%ss",op_src->str);break;
		case 3: print_asm("movw %s,%%ds",op_src->str);break;
		case 4: print_asm("movw %s,%%fs",op_src->str);break;
		case 5: print_asm("movw %s,%%gs",op_src->str);
	}
	return len + 1;
}

#endif

make_helper(concat(mov_a2moffs_, SUFFIX)) {
	swaddr_t addr = instr_fetch(eip + 1, 4);
	swaddr_write(addr, DATA_BYTE, (cpu.gpr[check_reg_index(R_EAX)]._32),DS);
	print_asm("mov" str(SUFFIX) " %%%s,0x%x", REG_NAME(R_EAX), addr);
	return 5;
}

make_helper(concat(mov_moffs2a_, SUFFIX)) {
	swaddr_t addr = instr_fetch(eip + 1, 4);
	REG(R_EAX) = MEM_R(addr);

	print_asm("mov" str(SUFFIX) " 0x%x,%%%s", addr, REG_NAME(R_EAX));
	return 5;
}


/* 
	and take "a2moffs" as example 
	we get the function:
	int mov_a2moffs_b{
		swaddr_t addr = instr_fetch(eip+1,4);
		swaddr_write(addr, 1 ,(cpu.gpr[check_reg_index(R_EAX)]._32));
		printt_asm("mov b %%%s,0x%x",regs_b[R_EAX],addr);
		return 5;
	}
*/
#include "cpu/exec/template-end.h"
