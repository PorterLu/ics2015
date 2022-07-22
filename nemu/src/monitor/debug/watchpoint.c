#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "cpu/reg.h"

#define NR_WP 32
CPU_state cpu;

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp()
{
	WP* pointer = free_;
	if(pointer != NULL)
	{
		free_ = pointer->next;
		pointer->next = head;
		head = pointer;
		return pointer;
	}
	
	assert(0);
}

void free_wp(int NO)
{
	WP* pointer = head;
	WP* pre = NULL;
	while(pointer != NULL)
	{
		if(pointer->NO == NO)
		{
			pre->next = pointer->next;
			pointer->next = free_;
			free_ = pointer;
			return ;
		}
		pre = pointer;
		pointer = pointer->next;
	}
}

void print_wp()
{
	printf("watchpoint list:\n");
    WP* pointer = head;
    while(pointer != NULL)
    {
            printf(" NO: %d, ",pointer->NO);
            printf(" expr: %s %d\n",pointer->expr,pointer->value);
            pointer = pointer->next;
    }


}

uint32_t check_wp()
{
	bool success =true,record = false;
	WP *pointer = head;
    while(pointer != NULL)
    {
            int value = expr(pointer->expr,&success);
            if(value != pointer->value)
            {
                    record = true;
                    pointer->value = value;
					printf("w%d,Program stop before 0x%08x\n",pointer->NO,cpu.eip);
                    break;
            }
            pointer = pointer->next;
    }
	return record;
}

