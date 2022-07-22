#include "common.h"
#include "memory/cache.h"
#include "cpu/reg.h"
#include "x86-inc/mmu.h"
#include "device/mmio.h"

//#define PAGE_SIZE (4<<10)
#define PAGE_DIR(addr) ((addr>>22)&0x3ff)
#define PAGE_M_OFFSET(addr)  ((addr&0x003ff000)>>12)
#define PAGE_OFFSET(addr)  (addr&0x00000fff)

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);
Cache* cache;
Cache* cache_l2;

lnaddr_t seg_translate(swaddr_t eip,uint32_t len, uint16_t sreg)
{

	//GD desc = dram_read(cpu.gdtr.base + ((cpu.sr[sreg])<<3) ,8);
	//cpu.sr[sreg].gdesc = desc;
	uint32_t base,limit;
	base = ((cpu.sr[sreg].gdesc>>32)&0xff000000) | ((cpu.sr[sreg].gdesc>>16) &0x00ff0000) |\
					((cpu.sr[sreg].gdesc>>16)&0x0000ffff);
	//printf("%x\n",base);

	limit = ((cpu.sr[sreg].gdesc>>32)&0x000f0000) | ((cpu.sr[sreg].gdesc & 0x0000ffff));
	if(cpu.sr[sreg].gdesc&0x0080000000000000)
		limit = (limit << 12) | 0x00000fff;
	//printf("%x\n",limit);
	assert((eip+len > eip) |( eip + len < limit));
	/*
	else
	{
		base = cpu.sr[sreg].sreg <<4;
		assert(eip <= 0xffff);
	}*/

	return base + eip;
	//return eip;
}

/* Memory accessing interfaces */

uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	int mmio_id = is_mmio(addr);
	//printf("len: %ld\n",len);
	//printf("here 4 mmio_id %d\n",mmio_id);
	//if(mmio_id >= 0)
		//printf("Yes,mmio_id >=0\n");
	if(mmio_id >= 0)
		return mmio_read(addr,len,mmio_id);
	else
	{
		//printf("here 5\n");
		return cache->read(cache, addr, len) & (~0u >> ((4 - len) << 3));
		//return dram_read(addr,len) & (~0u >> ((4 - len) << 3));
		//printf("here 6\n");
	}
	//printf("here 7\n");
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	int mmio_id = is_mmio(addr);
	if(mmio_id >= 0)
		mmio_write(addr,len,data,mmio_id);
	else
	{
		//printf("%x %ld\n",addr,len);
		cache->write(cache, addr, &data, len);
		//dram_write(addr,len,data);
	}
}

hwaddr_t page_translate(lnaddr_t addr)
{
	uint32_t page_dir_base = cpu.cr3.page_directory_base<<12;
	//int i = 0 ;
	//for(i=0;i<1024;i++)
		//printf("",*((page_dir_base + ))
	//printf("lnaddr:%x\n",addr);
	//printf("page dir base %x\n",page_dir_base);
	//printf("ecx :%x\n",cpu.ecx);
	uint32_t hwaddr = hwaddr_read(page_dir_base + PAGE_DIR(addr)*4,4);
	//PTE pte;
	//pte.val = hwaddr;
	//if(pte.present == 0)
	//	printf("Page does not exist\n");
	//else
	//	printf("Page exists\n");
	//printf("addr1: %x\n",hwaddr);
	uint32_t hwaddr2 = hwaddr_read((hwaddr&0xfffff000) + PAGE_M_OFFSET(addr)*4,4);
	//pte.val = hwaddr2; 
	
	//if(pte.present == 0)
	//{
	//	printf("Page does not exist\n");
	//	printf("cpu edi:%x\n",cpu.edi);
	//}
	//else
	//	printf("Page exists\n");
	
	//printf("addr2: %x\n",hwaddr2);

	//printf("%x\n",hwaddr+PAGE_OFFSET(addr));
	//printf("final address: %x\n\n",(hwaddr2&0xfffff000) + PAGE_OFFSET(addr));
	
	return (hwaddr2&0xfffff000) + PAGE_OFFSET(addr);	
}


uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	//printf("Here 2\n");

	if(cpu.cr0.protect_enable == 1 && cpu.cr0.paging == 1)
	{

		//printf("eip: %x\n",cpu.eip);
		//printf("I am reading\n");
		//printf("lnaddr:%x\n",addr);
		bool cross_page = (addr % PAGE_SIZE + len) <= PAGE_SIZE ? false : true;
		if(cross_page)
		{
			//printf("cross page\n");
			uint32_t i,j,data;
			uint32_t len1 = PAGE_SIZE - addr%PAGE_SIZE;
			uint8_t temp[4];

			data = hwaddr_read(page_translate(addr),len1);
			for(i = 0; i < len1;i++)
				temp[i] = *((uint8_t*)&data + i);
			if(len -len1 != 0)
			{
				data = hwaddr_read(page_translate(addr + len1),len - len1);
				for(j = i;j<len;j++)
					temp[j] =*((uint8_t*)&data + j-i);
			}
			//printf("\n\n");
			return *((uint32_t*)temp);
		}
		else
		{
			hwaddr_t hwaddr = page_translate(addr);
			//printf("physic address %x\n\n",hwaddr);
			return hwaddr_read(hwaddr, len);
		}
	}

	//printf("here 3\n");

	return hwaddr_read(addr,len);
	
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {

	if(cpu.cr0.protect_enable == 1 && cpu.cr0.paging == 1)
	{
		//printf("eip %x\n",cpu.eip);
		//printf("I am writing\n");
		//printf("lnaddr %x\n",addr);
		bool cross_page = (addr % PAGE_SIZE + len) <= PAGE_SIZE ? false : true;
		if(cross_page)
		{
			uint32_t i=0,j;
			uint32_t len1 = PAGE_SIZE - addr%PAGE_SIZE;

			for(;i<len1;i++)
			{
				uint8_t temp = *((uint8_t*)&data + i);
				hwaddr_write(page_translate(addr+i),1,temp);
			}		

			if(len - len1 != 0)
			{
				for(j = i;j<len;j++)
				{
					uint8_t temp = *((uint8_t*)&data + j);
					hwaddr_write(page_translate(addr+j),1,temp);
				}
			}
			//printf("\n\n");
		}
		else
		{
			hwaddr_t hwaddr = page_translate(addr);
			//printf("physic address %x\n\n",hwaddr);
			hwaddr_write(hwaddr,len,data);
		}
	}
	else
	{
		hwaddr_write(addr, len, data);
	}
}

uint32_t swaddr_read(swaddr_t addr, size_t len, uint16_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	//printf("Here 0\n");

	lnaddr_t lnaddr = addr;
	if(cpu.cr0.protect_enable == 1)
		lnaddr = seg_translate(addr, len, sreg);

  	return lnaddr_read(lnaddr, len);

	//return hwaddr_read(addr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data, uint16_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	//nemu没有完全模拟真实机器，实模式下是使用段寄存器的，但是现在nemu一开始就运行在32位，不使用段寄存器
	lnaddr_t lnaddr = addr;
	if(cpu.cr0.protect_enable == 1)
		 lnaddr = seg_translate(addr, len, sreg);
  	lnaddr_write(lnaddr, len,  data);
	//hwaddr_write(addr, len, data);
}

