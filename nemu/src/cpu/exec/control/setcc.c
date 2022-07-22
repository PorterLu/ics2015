#include "cpu/exec/helper.h"
#include "cpu/decode/modrm.h"

int seta(swaddr_t eip)
{
    int len = decode_rm_b(eip+1);
    if(eflags.CF == 0 && eflags.ZF == 0)
        write_operand_b(op_src,1);
    print_asm("seta" " %s",op_src->str);
    return len+1;
}

int setae(swaddr_t eip)
{
    int len = decode_rm_b(eip+1);
    if(eflags.CF == 0)
        write_operand_b(op_src,1);
    print_asm("setae" " %s",op_src->str);
    return len + 1;
}

int setb(swaddr_t eip)
{
    int len = decode_rm_b(eip+1);
    if(eflags.CF == 1)
        write_operand_b(op_src,1);
    print_asm("setb" " %s",op_src->str);
    return len+1;
}

int setbe(swaddr_t eip)
{
    int len = decode_rm_b(eip+1);
    if(eflags.CF == 1 || eflags.ZF == 1)
        write_operand_b(op_src,1);
    print_asm("setbe" " %s",op_src->str);
    return len+1;
}

int sete(swaddr_t eip)
{
    int len = decode_rm_b(eip+1);
    if(eflags.ZF == 1)
        write_operand_b(op_src,1);
    print_asm("sete" " %s",op_src->str);
    return len+1;
}

int setg(swaddr_t eip)
{
    int len = decode_rm_b(eip+1);
    if(eflags.ZF == 0 && eflags.SF == eflags.OF)
        write_operand_b(op_src,1);
    print_asm("setg" " %s",op_src->str);
    return len+1;
}

int setge(swaddr_t eip)
{
    int len = decode_rm_b(eip+1);
    if(eflags.SF == eflags.OF)
        write_operand_b(op_src,1);
    print_asm("setge" " %s",op_src->str);
    return len+1;
}

int setl(swaddr_t eip)
{
    int len = decode_rm_b(eip+1);
    if(eflags.SF != eflags.OF)
        write_operand_b(op_src,1);
    print_asm("setl" " %s",op_src->str);
    return len+1;
}

int setle(swaddr_t eip)
{
    int len = decode_rm_b(eip+1);
    if(eflags.SF != eflags.OF || eflags.ZF == 1)
        write_operand_b(op_src,1);
    print_asm("setle" " %s",op_src->str);
    return len+1;
}

int setne(swaddr_t eip)
{
    int len = decode_rm_b(eip+1);
    if(eflags.ZF == 0)
        write_operand_b(op_src,1);
    print_asm("setne" " %s",op_src->str);
    return len+1;
}

int setno(swaddr_t eip)
{
    int len = decode_rm_b(eip+1);
    if(eflags.OF == 0)
        write_operand_b(op_src,1);
    print_asm("setno" " %s",op_src->str);
    return len+1;
}

int setnp(swaddr_t eip)
{
    int len = decode_rm_b(eip+1);
    if(eflags.PF == 0)
        write_operand_b(op_src,1);
    print_asm("setnp" " %s",op_src->str);
    return len+1;
}

int setns(swaddr_t eip)
{
    int len = decode_rm_b(eip+1);
    if(eflags.SF == 0)
        write_operand_b(op_src,1);
    print_asm("sets" " %s",op_src->str);
    return len+1;
}

int seto(swaddr_t eip)
{
    int len = decode_rm_b(eip+1);
    if(eflags.OF == 1)
        write_operand_b(op_src,1);
    print_asm("seto" " %s",op_src->str);
    return len+1;
}

int setp(swaddr_t eip)
{
    int len = decode_rm_b(eip+1);
    if(eflags.PF == 1)
        write_operand_b(op_src,1);
    print_asm("setp" " %s",op_src->str);
    return len+1;
}

int sets(swaddr_t eip)
{
    int len = decode_rm_b(eip+1);
    if(eflags.SF == 1)
        write_operand_b(op_src,1);
    print_asm("seta" " %s",op_src->str);
    return len+1;
}

