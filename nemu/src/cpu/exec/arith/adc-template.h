 #include "cpu/exec/template-start.h"

 #define instr adc 
 EFLAGS_state eflags;

 static void do_execute(){
    DATA_TYPE result = op_dest->val + op_src->val + eflags.CF;
    eflags.CF=0,eflags.OF=0,eflags.SF=0,eflags.ZF=0,eflags.PF=0;
    if(MSB(result) == 1) eflags.SF = 1;
    if(result == 0) eflags.ZF = 1;
    int count=0,i=0,tmp=0x01;
    for(i=0;i<8;i++)
    { 
        if((tmp&result) == 1)
            count++;
        tmp = tmp << 1;
    }
    if(count%2 == 0) eflags.PF = 1;
    if(result<op_dest->val || result<op_src->val) eflags.CF = 1;
    if(!(MSB(op_dest->val)^MSB(op_src->val))&&(MSB(op_dest->val)^MSB(result))) eflags.OF = 1;
    OPERAND_W( op_dest , result);
    print_asm_template2();
 }

 make_instr_helper(i2a)
 make_instr_helper(i2rm)
 make_instr_helper(r2rm)
 make_instr_helper(rm2r)

 #if DATA_BYTE == 2 || DATA_BYTE == 4
 make_instr_helper(si2rm)
 #endif

 #include "cpu/exec/template-end.h"