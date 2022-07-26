#include "cpu/exec/helper.h"

make_helper(exec);

make_helper(rep) {
	int len;
	int count = 0;
	

	if(instr_fetch(eip + 1, 1) == 0xc3) {
		/* repz ret */
		exec(eip + 1);
		len = 0;
	}
	else {
		len = 1;
		while(cpu.ecx) {
			exec(eip + 1);
			count ++;
			cpu.ecx --;
			//printf("%d\n",cpu.ecx);
			//printf("ecx %x\n\n\n",cpu.ecx);
			assert(ops_decoded.opcode == 0xa4	// movsb
				|| ops_decoded.opcode == 0xa5	// movsw
				|| ops_decoded.opcode == 0xaa	// stosb
				|| ops_decoded.opcode == 0xab	// stosw
				|| ops_decoded.opcode == 0xa6	// cmpsb
				|| ops_decoded.opcode == 0xa7	// cmpsw
				|| ops_decoded.opcode == 0xae	// scasb
				|| ops_decoded.opcode == 0xaf	// scasw
				);
			//if(ops_decoded.opcode == 0xaa || ops_decoded.opcode == 0xab)

			if ((ops_decoded.opcode == 0xa6 || ops_decoded.opcode == 0xa7) && eflags.ZF == 0)
				break;
			/* TODO: Jump out of the while loop if necessary. */

		}
		
	}

	//printf("stosl over\n");

#ifdef DEBUG
	char temp[80];
	sprintf(temp, "rep %s", assembly);
	sprintf(assembly, "%s[cnt = %d]", temp, count);
#endif
	
	//printf("stosl over\n");
	return len + 1;
}
