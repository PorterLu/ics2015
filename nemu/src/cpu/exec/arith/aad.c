#include "cpu/exec/helper.h"
#include "cpu/decode/decode.h"

int aad(swaddr_t eip)
{
    decode_i_b(eip+1);
    int val = (uint8_t)op_src->val;
    uint8_t ah = (cpu.eax & 0x0000ff00) >> 8;
    cpu.eax = (cpu.eax & 0xffffff00) + ((ah * val + ( cpu.eax & 0xff ))&0xff);
    print_asm("aad %s",op_src->str);
    return 2; 
}
