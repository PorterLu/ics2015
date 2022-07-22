#include "common.h"
#include "memory.h"
#include "debug.h"
#include <string.h>
#include <elf.h>

#ifndef IA32_PAGE
#define IA32_PAGE
#endif

#define ELF_OFFSET_IN_DISK 0
#define RAMDISK_START ((void *)0)

#ifdef HAS_DEVICE
void ide_read(uint8_t *, uint32_t, uint32_t);
void ide_write(uint8_t *, uint32_t ,uint32_t);
void buf_writeback(void);
#else
void ramdisk_read(uint8_t *, uint32_t, uint32_t);
#endif

#define STACK_SIZE (1 << 20)

void create_video_mapping();
uint32_t get_ucr3();

uint32_t loader() {

	Elf32_Ehdr *elf;
	Elf32_Phdr *ph = NULL;

	uint8_t buf[4096];
	//printk("1\n");

#ifdef HAS_DEVICE
	
	ide_read(buf, ELF_OFFSET_IN_DISK, 4096);

#else
	ramdisk_read(buf, ELF_OFFSET_IN_DISK, 4096);
#endif

	elf = (void*)buf;

	/* TODO: fix the magic number with the correct one */
	const uint32_t elf_magic = 0x464c457f; //0x7f454c46
	uint32_t *p_magic = (void *)buf; 
	//Log("%d\n",*p_magic);
	nemu_assert(*p_magic == elf_magic);

	/* Load each program segment */
	//panic("please implement me");

	uint32_t e_phoff = elf->e_phoff;
	uint32_t e_phensize = elf->e_phentsize;
	uint32_t e_phnum = elf->e_phnum;

	//nemu_assert(e_phnum == 3);
		/* Scan the program header table, load each segment into memory */

	//uint8_t Ph_buf[e_phensize];
	int i;

	/*
	for(i=0;i<e_phnum;i++)
	{
		ph = RAMDISK_START + e_phoff + i*e_phensize;
		if(ph->p_type == PT_LOAD)
		{
			uint32_t p_vaddr = ph->p_vaddr;
			uint32_t p_offset = ph->p_offset;
			uint32_t p_filesz = ph->p_filesze;
			uint32_t p_memsz = ph->p_memsz;

			mm_malloc(p_vaddr,p_memsz);
		}
	}*/
	uint8_t data[65536*16];
	for (i=0 ;i< e_phnum; i++)
	{
		ph = (void*)buf + e_phoff + i*e_phensize;
		if(ph->p_type == PT_LOAD) 
		{
			//asm(".byte 0xd6");
			uint32_t  p_vaddr = ph->p_vaddr;
			uint32_t p_offset = ph->p_offset;
			uint32_t p_filesz = ph->p_filesz;
			uint32_t p_memsz = ph->p_memsz;

			//printk("\n");
			uint8_t* addr = (uint8_t*) mm_malloc(p_vaddr,p_memsz);
			printk("addr:%x vaddr:%x p_offset:%x p_memsz:%d p_filesz:%d\n",addr,p_vaddr,p_offset,p_memsz,p_filesz);

			ide_read(data,p_offset + ELF_OFFSET_IN_DISK,p_filesz);
			//asm(".byte 0xd6");

			//addr = addr + ph->p_filesz;
			//int j;
			//for(j=0;j<p_memsz-p_filesz;j++)
				//*(addr+j)=0;
			//ide_write(data,addr,p_filesz);
			//区域清零
			//nemu_assert(p_filesz == 0x7944 || p_filesz == 0x928);
			//printk("p_vaddr:%d , p_offset:%d ,p_filesz:%d ,p_memsz:%d \n",p_vaddr,p_offset,p_filesz,p_memsz);
		/*	uint32_t n,j,addr;
			n = p_filesz / 4096;
			for(j=0;j<n;j++)
			{
				ide_read(data,p_offset + j*4096,4096);
				addr =mm_malloc(p_vaddr + j*4096 ,4096);
				ide_write(data, addr ,4096);
			}

			ide_read(data,p_offset+j*4096,p_filesz - 4096*(j-1));
			ide_write(data,addr+j*4096,p_filesz-(j-1)*4096);*/
			//memset(RAMDISK_START + p_vaddr + p_filesz, 0, p_memsz - p_filesz);

			//加载segment
			//ramdisk_read((void*)addr, 0+ p_offset,p_filesz);
			//uint32_t j;
			//for(j=0;j<p_filesz;j++)
			memcpy(addr,data,p_filesz);

			/*
			int j;
			for(j=0;j<p_filesz;j++)
			{
				if(*((uint8_t*)RAMDISK_START + physic_addr + j) != *((uint8_t*)RAMDISK_START + p_offset + j))
					asm(".byte 0xd6");
			}*/


			/* TODO: read the content of the segment from the ELF file 
			* to the memory region [VirtAddr, VirtAddr + FileSiz)
			*/
		
			/* TODO: zero the memory region 
			* [VirtAddr + FileSiz, VirtAddr + MemSiz)
			*/
		}

#ifdef IA32_PAGE
			/* Record the program break for future use. */
			//设置堆空间申请的起始地址
			extern uint32_t brk;
			uint32_t new_brk = ph->p_vaddr + ph->p_memsz - 1;
			if(brk < new_brk) { brk = new_brk; }
#endif

	}

	volatile uint32_t entry = elf->e_entry;

#ifdef IA32_PAGE
	mm_malloc(KOFFSET - STACK_SIZE, STACK_SIZE);


#ifdef HAS_DEVICE

	create_video_mapping();
#endif
	//asm(".byte 0xd6");
	//cpu.eip += 0xc0000000;
	uint32_t val = get_ucr3();
	write_cr3(val);
	//asm volatile("movl %0, %%cr3" : : "a"(val));
	//asm(".byte 0xd6");
#endif

	return entry;
}
