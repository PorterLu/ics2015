#include "common.h"
#include "x86.h"
#include "memory.h"
#include <string.h> 

static PDE kpdir[NR_PDE] align_to_page;						// kernel page directory
static PTE kptable[PHY_MEM / PAGE_SIZE] align_to_page;		// kernel page tables

PDE* get_kpdir() { return kpdir; }

/* set up page tables for kernel */
void init_page(void) {
	//asm(".byte 0xd6");
	CR0 cr0;
	CR3 cr3;
	//asm (".byte 0xd6");
	//if((uint32_t)kptable == 0xc013a000)
	PDE *pdir = (PDE *)(kpdir - 0xc0000000);
	PTE *ptable = (PTE *)(kptable - 0xc0000000);

	//if(va_to_pa(kpdir) == pdir)
	//	asm (".byte 0xd6");

	uint32_t pdir_idx;
	/* make all PDEs invalid */
	memset(pdir, 0, NR_PDE * sizeof(PDE));
	//asm (".byte 0xd6");

	/* fill PDEs */

	for (pdir_idx = 0; pdir_idx < 32; pdir_idx ++) {
		pdir[pdir_idx].val = ((uint32_t)ptable)|0x7;
		pdir[pdir_idx + 768].val = ((uint32_t)ptable)|0x7;
		ptable += NR_PTE;

	}

	/* fill PTEs */

	/* We use inline assembly here to fill PTEs for efficiency.
	 * If you do not understand it, refer to the C code below.
	 */

	//asm (".byte 0xd6");
	asm volatile ("std;\
	 1: stosl;\
		subl %0, %%eax;\
		jge 1b" : : 
		"i"(PAGE_SIZE), "a"((PHY_MEM - PAGE_SIZE) | 0x7), "D"(ptable - 1));


	/*
		===== referenced code for the inline assembly above =====

		uint32_t pframe_addr = PHY_MEM - PAGE_SIZE;
		ptable --;

		// fill PTEs reversely
		for (; pframe_addr >= 0; pframe_addr -= PAGE_SIZE) {
			ptable->val = make_pte(pframe_addr);
			ptable --;
		}
	*/

	//asm(".byte 0xd6");

	/* make CR3 to be the entry of page directory */
	cr3.val = 0;
	cr3.page_directory_base = ((uint32_t)pdir) >> 12;
	write_cr3(cr3.val);

	/* set PG bit in CR0 to enable paging */
	cr0.val = read_cr0();
	cr0.paging = 1;
	write_cr0(cr0.val);

}

/* GDT in the kernel's memory, whose virtual memory is greater than 0xC0000000. */
static SegDesc gdt[NR_SEGMENTS];

static void
set_segment(SegDesc *ptr, uint32_t pl, uint32_t type) {
	ptr->limit_15_0  = 0xFFFF;
	ptr->base_15_0   = 0x0;
	ptr->base_23_16  = 0x0;
	ptr->type = type;
	ptr->segment_type = 1;
	ptr->privilege_level = pl;
	ptr->present = 1;
	ptr->limit_19_16 = 0xF;
	ptr->soft_use = 0;
	ptr->operation_size = 0;
	ptr->pad0 = 1;
	ptr->granularity = 1;
	ptr->base_31_24  = 0x0;
}

/* This is similar with the one in start.S. However the previous one
 * cannot be accessed in user process, because its virtual address is
 * below 0xC0000000, and is not in the user process' address space. */
void
init_segment(void) {
	memset(gdt, 0, sizeof(gdt));
	set_segment(&gdt[SEG_KERNEL_CODE], DPL_KERNEL, SEG_EXECUTABLE | SEG_READABLE);
	set_segment(&gdt[SEG_KERNEL_DATA], DPL_KERNEL, SEG_WRITABLE );

	write_gdtr(gdt, sizeof(gdt));//加载到高位地址
}

