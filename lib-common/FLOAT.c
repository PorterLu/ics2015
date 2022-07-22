#include "FLOAT.h"

typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;


FLOAT F_mul_F(FLOAT a, FLOAT b) {
    	long long value = (a * b)/65536;
	return (int)value;

}

FLOAT F_div_F(FLOAT a, FLOAT b) {
    int sign = 1;
    if (a < 0) {
        sign *= -1;
        a = -a;
    }
    if (b < 0) {
        sign *= -1;
        b = -b;
    }
    int res = a / b;
    a = a % b;
    int i;
    for ( i = 0; i < 16; i++) {
        a <<= 1;
        res <<= 1;
        if (a >= b) {
            a -= b;
            res++;
        }
    }
 
	int c = F2int(b);
	int value = F_div_int(a,c);
    return value;
}

/*
FLOAT f2F(float a) {

    uint32_t af = *(uint32_t *)&a;
    uint32_t sign = af >> 31;
    int exp = (af >> 23) & 0xff;
    uint32_t sig = af & 0x7fffff;
    if (exp != 0) sig += 1 << 23;
    exp -= 150;
    if (exp < -16) sig >>= -16 - exp;
    if (exp > -16) sig <<= exp + 16;
    return sign == 0 ? sig : -sig;

	long long b = a;
	int c = b * 65536;
	return c;
}*/

/*
FLOAT F_mul_F(FLOAT a, FLOAT b) {
 long long c = (long long)a * (long long)b;
 return (FLOAT)(c >> 16);
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
 FLOAT p, q;
 asm volatile("idiv %2" : "=a"(p), "=d"(q) : "r"(b), "a"(a << 16), "d"(a >> 16));
 return p;
}*/


FLOAT f2F(float a) {
/*	
	int value;
	asm  volatile("mov %%eax,%%ebx;"  \
  		"mov %%eax,%%ecx;" \
               	"and 0x7fffffff,%%eax;"  \
		"mov 0x1000000,%%edx;"\
               	"div %%edx;"  \
               	"add 0x2000000,%%ecx;" \
		"push %%eax;"\
		"mov %%ecx,%%eax;"\
		"mov 0x1000000,%%edx;"
               	"div %%edx;" \
		"mov %%eax,%%ebx;"\
		"pop %%eax;"\
               	"sub 0x7f,%%eax;" \
               	"and 0xff,%%eax;" \
               	"mul %%ebx;":"=a"(value):"0"(a));*/
/*
 int sign = b >> 31;
 int exp = (b >> 23) & 0xff;
 FLOAT c = b & 0x7fffff;
 if (exp != 0) {
  c += 1 << 23;
 }
 exp -= 150;
 if (exp < -16) {
  c >>= -16 - exp;
 }
 if (exp > -16) {
  c <<= exp + 16;
 }*/

		FLOAT my_a=*(FLOAT *)&a;
	FLOAT my_mantissa=(my_a&0x007fffff)|0x00800000;
	FLOAT my_order=(my_a&0x7f800000)>>23;
	FLOAT my_move=my_order-127;
	if(my_move>=7){
		if(my_a>0)
			return (my_mantissa<<(my_move-7));
		else
			return -(my_mantissa<<(my_move-7));
	}
	else{
		if(my_a>0)
			return (my_mantissa>>(7-my_move));
		else
			return -(my_mantissa>>(7-my_move));
	}


 return my_a;
}

FLOAT Fabs(FLOAT a) {
 FLOAT b;
 if (a > 0){
  b = a;
 } else {
  b = -a;

 }
 return b;
}

/*
FLOAT Fabs(FLOAT a) {
    return a >= 0 ? a : -a;
}*/

FLOAT sqrt(FLOAT x) {
    FLOAT dt, t = int2F(2);

    do {
        dt = F_div_int((F_div_F(x, t) - t), 2);
        t += dt;
    } while(Fabs(dt) > f2F(1e-4));

    return t;
}

FLOAT pow(FLOAT x, FLOAT y) {
    /* we only compute x^0.333 */
    FLOAT t2, dt, t = int2F(2);

    do {
        t2 = F_mul_F(t, t);
        dt = (F_div_F(x, t2) - t) / 3;
        t += dt;
    } while(Fabs(dt) > 0.01); //f2F(1e-4)

    return t;
}
