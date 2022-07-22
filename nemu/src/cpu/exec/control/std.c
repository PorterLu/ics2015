#include "cpu/exec/helper.h"

uint32_t std(swaddr_t eip)
{
	eflags.DF = 1;
	print_asm("std");
	return 1;
}
