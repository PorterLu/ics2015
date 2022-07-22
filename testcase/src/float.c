#include"trap.h"
#include "FLOAT.h"
int main(void)
{
	//a = a+1;
	int b = 1.2 * (2<<16);
	b = b + 1;
	HIT_GOOD_TRAP;
}
