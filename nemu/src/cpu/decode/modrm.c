#include "cpu/decode/modrm.h"
#include "cpu/helper.h"

/*
typedef union {
	struct {
		uint8_t R_M		:3;
		uint8_t reg		:3;
		uint8_t mod		:2;
	};
	struct {
		uint8_t dont_care	:3;
		uint8_t opcode		:3;
	};
	uint8_t val;
} ModR_M;

typedef union {
	struct {
		uint8_t base	:3;
		uint8_t index	:3;
		uint8_t ss		:2;
	};
	uint8_t val;
} SIB;

	ModR/M                        SIB
	--------------------          -----------------
	|  3   |  3  |  2  |  		  |	3   | 3   | 2 | 
	|      |     |	   |		  |     |     |   |
	| R_M  | reg | mod | 		  | base|index| ss|
	—-------------------          -----------------   
	
	load_addr是在m的mod域不为3的时候执行，那么如果m的R_M域为4的话，那么就要读SIB，取一个字节作为SIB，
	ss为比例因子，base为基地址寄存器，index不能为4，其他直接赋值

	否则说明没有SIB，m中的R_M就是基地址寄存器号
	


 */
int load_addr(swaddr_t eip, ModR_M *m, Operand *rm) {//这里的eip是取完opcode后的地址
	assert(m->mod != 3);
	
	int32_t disp;
	int32_t instr_len, disp_offset, disp_size = 4;
	int base_reg = -1, index_reg = -1, scale = 0;
	swaddr_t addr = 0;

	if(m->R_M == R_ESP) {
		SIB s;
		s.val = instr_fetch(eip + 1, 1);
		base_reg = s.base;
		disp_offset = 2;
		scale = s.ss;

		if(s.index != R_ESP) { index_reg = s.index; }
	}
	else {
		/* no SIB */
		base_reg = m->R_M;
		disp_offset = 1;
	}

	if(m->mod == 0) {
		if(base_reg == R_EBP) { base_reg = -1; }//直接寻址
		else { disp_size = 0; }
	}
	else if(m->mod == 1) { disp_size = 1; }

	//取偏移
	instr_len = disp_offset;
	if(disp_size != 0) {
		/* has disp */
		disp = instr_fetch(eip + disp_offset, disp_size);
		if(disp_size == 1) { disp = (int8_t)disp; }

		instr_len += disp_size;
		addr += disp;
	}

	if(base_reg != -1) {
		addr += reg_l(base_reg);
	}

	if(index_reg != -1) {
		addr += reg_l(index_reg) << scale;
	}

#ifdef DEBUG
	char disp_buf[16];
	char base_buf[8];
	char index_buf[8];

	if(disp_size != 0) {
		/* has disp */
		sprintf(disp_buf, "%s%#x", (disp < 0 ? "-" : ""), (disp < 0 ? -disp : disp));
	}
	else { disp_buf[0] = '\0'; }

	if(base_reg == -1) { base_buf[0] = '\0'; }
	else { 
		sprintf(base_buf, "%%%s", regsl[base_reg]); 
	}

	if(index_reg == -1) { index_buf[0] = '\0'; }
	else { 
		sprintf(index_buf, ",%%%s,%d", regsl[index_reg], 1 << scale); 
	}

	if(base_reg == -1 && index_reg == -1) {
		sprintf(rm->str, "%s", disp_buf);
	}
	else {
		sprintf(rm->str, "%s(%s%s)", disp_buf, base_buf, index_buf);
	}
#endif

	rm->type = OP_TYPE_MEM;
	rm->addr = addr;
	rm->so.addr = addr;
	
	if(cpu.cr0.protect_enable == 1)
	{
		if(base_reg == 4 || base_reg == 5 || index_reg == 4 || index_reg == 4)
			rm->so.sreg = 2;
		else
			rm->so.sreg = 1;
	}


	return instr_len;
}

/* 
这里对read_ModR_M 进行解读 
首先三个参数分别是内存地址eip，操作数 rm,操作数 reg
typedef struct {
	uint32_t type;
	size_t size;
	union {
		uint32_t reg;
		swaddr_t addr;
		uint32_t imm;
		int32_t simm;
	};
	uint32_t val;
	char str[OP_STR_SIZE];
} Operand;

typedef struct {
	uint32_t opcode;
	bool is_operand_size_16;
	Operand src, dest, src2;
} Operands;

首先在eip处读取一个字节，将它整个赋值给ModR/M，
首先确定operand reg的类型为寄存器（？），reg的寄存器号为m的reg位

如果m的mod位为3，那么另一个操作数rm的类型一定是寄存器，它的寄存器号由R_M号决定。根据rm的数据大小对rm的val进行赋值
为什么不对reg的val赋值？因为rm的值-》reg

接下来所有的指令类型将都要访问内存，所以执行load_addr,传入的参数为eip，modrm，operand m

*/
int read_ModR_M(swaddr_t eip, Operand *rm, Operand *reg) {
	ModR_M m;
	m.val = instr_fetch(eip, 1);
	reg->type = OP_TYPE_REG;
	reg->reg = m.reg;

	if(m.mod == 3) {
		rm->type = OP_TYPE_REG;
		rm->reg = m.R_M;
		switch(rm->size) {
			case 1: rm->val = reg_b(m.R_M); break;
			case 2: rm->val = reg_w(m.R_M); break;
			case 4: rm->val = reg_l(m.R_M); break;
			default: assert(0);
		}
#ifdef DEBUG
		switch(rm->size) {
			case 1: sprintf(rm->str, "%%%s", regsb[m.R_M]); break;
			case 2: sprintf(rm->str, "%%%s", regsw[m.R_M]); break;
			case 4: sprintf(rm->str, "%%%s", regsl[m.R_M]); break;
		}
#endif
		return 1;
	}
	else {
		int instr_len = load_addr(eip, &m, rm);
		rm->val = swaddr_read(rm->addr, rm->size, op_dest->so.sreg);

		return instr_len;
	}
}

