#include "memory/cache.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include "cpu/reg.h"
//读取或者写的长度为1或者2或者4

extern Cache* cache;
extern Cache* cache_l2;

Cache* cache;
Cache* cache_l2;

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

void init_default(Cache *self){
    uint32_t block_num = CACHE_SIZE / BLOCK_SIZE;
    self->block = malloc(sizeof(Block) * block_num);
    int i;
    for(i=0;i<block_num;i++)
        self->block[i].valid = false;
    self->set_way = SET_WAY;
    self->cache_size = CACHE_SIZE;
}

void init_dirty_L2(Cache *self)
{
    uint32_t block_num = CACHE_SIZE_L2 / BLOCK_SIZE;
    self->block = malloc(sizeof(Block) *block_num);
    int i;
    for(i=0;i<block_num;i++)
    {
        //printf("%d ",i);
        self->block[i].valid = false;
        self->block[i].dirty = false;
    }
    self->set_way = SET_WAY_L2;
    self->cache_size = CACHE_SIZE_L2;
}

uint32_t read_default(Cache *self,uint32_t addr,uint32_t len){
    int32_t index = self->map_algo(self,addr),i,j,data;
    uint8_t tmp_data[4];
    bool cborder = addr%BLOCK_SIZE + len > BLOCK_SIZE? true : false;
    uint32_t len1 = cborder? BLOCK_SIZE - addr % BLOCK_SIZE : len;
    //printf("%x %d ",addr,len);
    if(index == -1 || self->block[index].valid == false)//L1未命中
    {
        if(index == -1)
            index = self->replace_algo(self,addr);
        for(i=0;i<BLOCK_SIZE;i++)
        {    
            uint8_t temp = cache_l2->read(cache_l2,addr/BLOCK_SIZE * BLOCK_SIZE + i,1);
            //uint8_t temp = dram_read(addr/BLOCK_SIZE * BLOCK_SIZE + i,1); 
            self->block[index].space[i] = temp;
        }
        self->block[index].valid = true;
        self->block[index].tag = addr/BLOCK_SIZE/((self->cache_size/BLOCK_SIZE)/self->set_way);
        
        
        //统计，如何计算总共的cycle
    }

    for(i=0; i< len1;i++)
        tmp_data[i]= self->cache_read(self,index,addr%BLOCK_SIZE + i);


    if(cborder)
    {
        data = read_default(self,addr+len1,len - len1);
        for(j=0;j<len-len1;j++,i++)
            tmp_data[i] = *((uint8_t*)&(data) + j);
    }

    //printf("cborder = %d ,%x %x %x %x\n",cborder,tmp_data[0],tmp_data[1],tmp_data[2],tmp_data[3]);
    //printf("%x \n",*((uint32_t*) tmp_data));
    return *((uint32_t*) tmp_data);
}

uint8_t cache_read_default(Cache *self,uint32_t bid,uint32_t offset){
    return self->block[bid].space[offset];
}

uint32_t group_rand_replace(Cache *self,swaddr_t addr)
{
    srand(time(0));
    int index = rand() % self->set_way;
    int group_section = (self->cache_size / BLOCK_SIZE)/self->set_way;
    int group_index = (addr/BLOCK_SIZE) % group_section;
    return  group_index * self->set_way + index;
}

uint32_t group_mapping(Cache *self,swaddr_t addr){
    int group_section = (self->cache_size / BLOCK_SIZE)/self->set_way;
    int group_index = (addr / BLOCK_SIZE) % group_section;
    int i;
    for(i = 0;i<self->set_way;i++)
    {
        int id = group_index * self->set_way + i;
        if(self->block[id].tag == addr/(BLOCK_SIZE*group_section))
            return id;
    }

    for(i = 0;i<self->set_way;i++)
    {
        int id = group_index * self->set_way + i;
        if(self->block[id].valid == false)
            return id;; 
    }

    return -1;
}

void cache_write_default(Cache *self,uint32_t bid,uint8_t data,uint32_t offset)
{
    self->block[bid].space[offset] = data;
}

void write_default(Cache *self,uint32_t addr,void* data ,uint32_t len)
{
    //先映射，知道是否命中，再看地址是否越过了一个cache块，如果是的，则检查第二个块是否命中，命中正常写
    //不命中调用写为中函数
    int32_t index,i=0;
    bool cborder = ((addr%BLOCK_SIZE) + len )> BLOCK_SIZE ? true : false;
    uint32_t len1 =cborder? BLOCK_SIZE - addr%BLOCK_SIZE : len;
    index = self->map_algo(self,addr);
    //printf("addr:%x   len1:%d\n",addr,len1);
    if(index == -1 || self->block[index].valid == false)
    {
        self->write_defect(self,addr,data,len1);
    }
    else
    {
        for(;i<len1;i++)
        {
            //write cache
            self->cache_write(self,index,*((uint8_t*)data + i),addr%BLOCK_SIZE + i);
            //write memory
            //dram_write(addr + i,1,(uint32_t)(*((uint8_t*)data + i)));
        }

        cache_l2->write(cache_l2,addr,(uint8_t*)data , len1);

    }

    if(cborder)
        write_default(self,addr+len1,(uint8_t*)data+len1,len-len1);
}

void write_defect_straight(Cache *self,swaddr_t addr, void* data,uint32_t len)
{
    //int i;
    //for(i=0;i<len;i++)
    //{
        //dram_write(addr+i,1,(uint32_t)(*((uint8_t*)data+i)));
    cache_l2->write(cache_l2,addr,(uint8_t*)data , len);
        //printf("%x ",*((uint8_t*)data + i));
    //}
    //printf("\n");
}



uint32_t read_writeback(Cache *self,uint32_t addr,uint32_t len)
{
    int32_t index = self->map_algo(self,addr),i,j,data;
    uint8_t tmp_data[4];
    bool cborder = addr%BLOCK_SIZE + len > BLOCK_SIZE? true : false;
    uint32_t len1 = cborder? BLOCK_SIZE - addr % BLOCK_SIZE : len;
    //printf("%x %d ",addr,len);
    if(index == -1 || self->block[index].valid == false)
    {
        if(index == -1)
        {
            index = self->replace_algo(self,addr);
            
            if(self->block[index].dirty == true)
            {
                //uint32_t group_offset = 0,set_way_t = self->set_way;
                //set_way_t -= 1;
                uint32_t addr_base = ((index/(self->set_way))<<(BLOCK_OFFSET))| \
                                        ((self->block[index].tag<<BLOCK_OFFSET)*((self->cache_size/BLOCK_SIZE)/self->set_way));
                for(i=0;i<BLOCK_SIZE;i++)
                    dram_write(addr_base+i,1,self->block[index].space[i]);
                self->block[index].dirty = false;
            }
        }
        for(i=0;i<BLOCK_SIZE;i++)
        {
            uint8_t temp = dram_read(addr/BLOCK_SIZE * BLOCK_SIZE + i,1); 
            self->block[index].space[i] = temp;
        }
        self->block[index].valid = true;
        self->block[index].tag = addr/BLOCK_SIZE/((self->cache_size/BLOCK_SIZE)/self->set_way);

        //统计
        /*
        if(cpu.eip > 0x800000)
        {
            cycle += 200;
            unhit += 1;
            read_unhit += 1;
        }*/
    }

    for(i=0; i< len1;i++)
        tmp_data[i]= self->cache_read(self,index,addr%BLOCK_SIZE + i);
    /*
    if(cpu.eip > 0x800000)
    {
        cycle += 2;
        hit += 1;
        read_hit +=1;
    }*/

    if(cborder)
    {
        data = read_default(self,addr+len1,len - len1);
        for(j=0;j<len-len1;j++,i++)
            tmp_data[i] = *((uint8_t*)&(data) + j);
    }

    //printf("cborder = %d ,%x %x %x %x\n",cborder,tmp_data[0],tmp_data[1],tmp_data[2],tmp_data[3]);
    return *((uint32_t*) tmp_data);

} 

void write_writeback(Cache *self,uint32_t addr,void* data ,uint32_t len){
    uint32_t index,i=0;
    bool cborder = addr%BLOCK_SIZE + len > BLOCK_SIZE ? true : false;
    index = self->map_algo(self,addr);
    uint32_t len1 = cborder? BLOCK_SIZE - addr%BLOCK_SIZE : len;
    //写命中，则直接写cache；写不命中，则调入cache，进行写
    
    if(index == -1 || self->block[index].valid == false)
        self->write_defect(self,addr,data,len1);
    else
    {
        for(;i<len1;i++)
            self->cache_write(self,index,*((uint8_t*)data + i),addr%BLOCK_SIZE + i);
        self->block[index].dirty = true;
    }

    if(cborder)
        write_writeback(self,addr+len1,(uint8_t*)data+len1,len-len1);
}

void write_defect_allocate(Cache *self,swaddr_t addr,void* data,uint32_t len)
{
    uint32_t index = self->map_algo(self,addr),i;
    if(index == -1) //要替换
    {
        //用随机替换算法可以找出要替换的块，根据脏位可以判断是否需要写入内存，然后将需要的块写入cache，进行写操作。
        index = self->replace_algo(self,addr);
        if(self->block[index].dirty == true)
        {
            uint32_t addr_base = (index/(self->set_way))<<(BLOCK_OFFSET)| \
                                    ((self->block[index].tag<< BLOCK_OFFSET)*((self->cache_size/BLOCK_SIZE)/self->set_way));
            for(i=0;i<BLOCK_SIZE;i++)
                dram_write(addr_base+i,1,self->block[index].space[i]);
            self->block[index].dirty = false;
        }

    }
    for(i=0;i<BLOCK_SIZE;i++)
    {
        uint8_t temp = dram_read(addr/BLOCK_SIZE * BLOCK_SIZE + i,1); 
        self->block[index].space[i] = temp;
    }
    self->block[index].valid = true;
    self->block[index].tag = addr/BLOCK_SIZE/((self->cache_size/BLOCK_SIZE)/self->set_way);

    for(i=0;i<len;i++)
        self->cache_write(self,index,*((uint8_t*)data+i),addr%BLOCK_SIZE + i);
    self->block[index].dirty = true;

}



void init_L2()
{
    cache_l2 = malloc(sizeof(Cache));
    cache_l2->cache_read = cache_read_default;
    cache_l2->cache_write = cache_write_default;
    cache_l2->map_algo = group_mapping;
    cache_l2->replace_algo = group_rand_replace;
    cache_l2->read = read_writeback;
    cache_l2->write = write_writeback;
    cache_l2->write_defect = write_defect_allocate;
    cache_l2->init = init_dirty_L2;
    cache_l2->init(cache_l2);

    cycle_l2 = 0;
    hit_l2 = 0;
    unhit_l2 = 0;
    write_hit = 0;
    write_unhit = 0;
    read_hit = 0;
    read_unhit = 0;
}

void init_cache(){
	cache = malloc(sizeof(Cache));
	cache->cache_read = cache_read_default;
	cache->cache_write = cache_write_default;
	cache->map_algo = group_mapping;
	cache->replace_algo = group_rand_replace;
	cache->write_defect = write_defect_straight;
	cache->read = read_default;
	cache->write = write_default;
	cache->init = init_default;
	cache->init(cache);

    cycle = 0;
    hit = 0;
    unhit = 0;
    write_hit = 0; 
    write_unhit = 0;
    read_hit = 0;
    read_unhit = 0;    
}

