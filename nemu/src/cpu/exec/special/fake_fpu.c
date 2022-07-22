#include "cpu/exec/helper.h"
#include "cpu/reg.h"
make_helper(fake_fpu) {
    
    uint8_t instr = instr_fetch(cpu.eip, 1);
    
    
    static int fldz_flag = 0;
    static int fstpl_flag = 0;

    int len = 0;
    switch (instr) {
        case 0xd9:
            print_asm("fldz (ignored)");
            if (!fldz_flag) {
                printf(" fldz at %04x:%08x, ignored\n",  cpu.cs.sreg, cpu.eip);
                fldz_flag = 1;
            }
            len = 2;
            break;
        case 0xdd:
            print_asm("fstpl  -0xf0(%%ebp) (ignored)");
            if (!fstpl_flag) {
                printf(" fstpl at %04x:%08x, ignored\n",cpu.cs.sreg, cpu.eip);
                fstpl_flag = 1;
            }
            len = 6;
            break;
    }

	return len;
}
