#include "cpu/exec/helper.h"
#include "cpu/reg.h"
#include "x86-inc/cpu.h"
#include <setjmp.h>
#include <string.h>

extern jmp_buf jbuf;
extern char asm_buf[128];
void print_bin_instr(swaddr_t eip, int len);
uint32_t global_n;

extern void raise_intr(uint8_t NO)
{
    //push the eflags,cs,eip by hardware
    swaddr_write(cpu.esp-4,4,(uint32_t)(uint8_t)eflags.val,SS);
    swaddr_write(cpu.esp-8,4,(uint32_t)(uint16_t)cpu.cs.sreg,SS);
    swaddr_write(cpu.esp-12,4,cpu.eip,SS);
    cpu.esp -= 12;

    //interupt handler finding
    uint16_t selector;
    uint32_t offset,temp;

    temp = lnaddr_read(cpu.idtr.base + NO*8, 4);
    selector =(temp&0xffff0000)>>16;
    offset = temp&0x0000ffff;
    temp = lnaddr_read(cpu.idtr.base + 4 + NO*8, 4);
    offset = (temp&0xffff0000)|offset;

    cpu.cs.sreg = selector;
    temp = lnaddr_read(cpu.gdtr.base + selector,4);

    cpu.cs.gdesc = (uint32_t)temp;
    temp = lnaddr_read(cpu.gdtr.base + selector + 4,4);
    cpu.cs.gdesc = (((uint64_t)temp)<<32)|cpu.cs.gdesc;
    cpu.eip = offset;

    //printf("%x %x\n",selector,offset);
    longjmp(jbuf,1);

}

lnaddr_t seg_translate(swaddr_t eip,uint32_t len ,uint8_t sr);
uint32_t page_translate(lnaddr_t eip);
int intr(swaddr_t eip)
{
    uint32_t intr_no = instr_fetch(eip+1,1);
    print_asm("int 0x%x",intr_no);
    int instr_len = 2;
    print_bin_instr(eip , instr_len);
	//printf("one instruction over\n");
	strcat(asm_buf, assembly);
	Log_write("%s\n", asm_buf);
    //printf("%s\n", asm_buf);
    /*
    if( global_n < 5)
    {
        printf("%d\n", global_n);
        printf( "%s\n", asm_buf );
    }*/

    cpu.eip += 2;
    //printf("%x\n",cpu.eip);
    //printf("%x\n",seg_translate(page_translate(cpu.eip),1,1));
    raise_intr(intr_no);
    return 2;
}
