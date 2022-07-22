#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"
#define expr_maxl 100

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;
	char expr[expr_maxl];
	uint32_t value;
	/* TODO: Add more members if necessary */


} WP;


WP* new_wp();
void free_wp(int NO);
void print_wp();
uint32_t check_wp();
#endif
