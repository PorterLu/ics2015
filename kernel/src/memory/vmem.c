#include "common.h"
#include "memory.h"
#include <string.h>

#define VMEM_ADDR 0xa0000
#define SCR_SIZE (320 * 200)
extern PDE updir[NR_PDE];
static PTE uptable_vm[NR_PTE] align_to_page;
/* Use the function to get the start address of user page directory. */
PDE* get_updir();


void create_video_mapping() {
	/* TODO: create an identical mapping from virtual memory area 
	 * [0xa0000, 0xa0000 + SCR_SIZE) to physical memory area 
	 * [0xa0000, 0xa0000 + SCR_SIZE) for user program. You may define
	 * some page tables to create this mapping.
	 */
	//panic("please implement me");
	//page_num需要向上取整
	int page_num = (SCR_SIZE + PAGE_SIZE - 1)/PAGE_SIZE,i=0;
	uint32_t addr = VMEM_ADDR;
	for(i = 0;i<page_num;i++)
	{
		uint32_t dir = (addr >> 22)&0x3ff;
		uint32_t page = (addr>> 12)&0x3ff;
		updir[dir].val = make_pde(va_to_pa(uptable_vm));
		uptable_vm[page].val = make_pte(addr);
		addr += PAGE_SIZE;
	}
}

void video_mapping_write_test() {
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for(i = 0; i < SCR_SIZE / 4; i ++) {
		buf[i] = i;
	}
}

void video_mapping_read_test() {
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for(i = 0; i < SCR_SIZE / 4; i ++) {
		assert(buf[i] == i);
	}
}

void video_mapping_clear() {
	memset((void *)VMEM_ADDR, 0, SCR_SIZE);
}

