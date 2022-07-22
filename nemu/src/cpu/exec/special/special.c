#include "cpu/exec/helper.h"
#include "monitor/monitor.h"
#include "memory/cache.h"
#define MAX_LEN 100

make_helper(inv) {
	/* invalid opcode */

	uint32_t temp[8];
	temp[0] = instr_fetch(eip, 4);
	temp[1] = instr_fetch(eip + 4, 4);

	uint8_t *p = (void *)temp;
	printf("invalid opcode(eip = 0x%08x): %02x %02x %02x %02x %02x %02x %02x %02x ...\n\n", 
			eip, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);

	extern char logo [];
	printf("There are two cases which will trigger this unexpected exception:\n\
1. The instruction at eip = 0x%08x is not implemented.\n\
2. Something is implemented incorrectly.\n", eip);
	printf("Find this eip value(0x%08x) in the disassembling result to distinguish which case it is.\n\n", eip);
	printf("\33[1;31mIf it is the first case, see\n%s\nfor more details.\n\nIf it is the second case, remember:\n\
* The machine is always right!\n\
* Every line of untested code is always wrong!\33[0m\n\n", logo);

	assert(0);
}

make_helper(nemu_trap) {
	print_asm("nemu trap (eax = %d)", cpu.eax);
	char buf[MAX_LEN];
	uint32_t len,i;
	switch(cpu.eax) {
		case 2:
			//printf("here\n");
			len = cpu.edx;
			i = 0;
			for(;i<len;i++)
			{
				char temp = swaddr_read(cpu.ecx + i,1,DS);
				buf[i] = temp;
			}
			printf("%s",buf);
			print_asm("int 0x80");
		   	break;

		default:
			printf("\33[1;31mnemu: HIT %s TRAP\33[0m at eip = 0x%08x\n\n",
					(cpu.eax == 0 ? "GOOD" : "BAD"), cpu.eip);
			
			if(cpu.eax == 0)
			{
				printf("cycle      write      read       hit        miss\n");
				printf("%-10ld %-10d %-10d %-10d %-10d\n",cycle,write_hit + write_unhit,\
															read_hit + read_unhit,\
															write_hit+read_hit,write_unhit+read_unhit);
			}
			nemu_state = END;
	}
	//asm(".byte 0xd6");
	return 1;
}

