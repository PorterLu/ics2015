#ifndef __CACHE_H__
#define __CACHE_H__

#include "common.h"

#define BLOCK_OFFSET 6
#define BLOCK_SIZE 64
#define CACHE_SIZE (64 << 10)
#define SET_WAY 8

#define CACHE_SIZE_L2 (1 << 22)
#define SET_WAY_L2 16

extern uint64_t cycle ;
extern uint32_t hit;
extern uint32_t unhit ;
extern uint32_t write_hit ;
extern uint32_t write_unhit ;
extern uint32_t read_hit ;
extern uint32_t read_unhit ;
extern uint64_t cycle_l2 ;
extern uint32_t hit_l2;
extern uint32_t unhit_l2;
extern uint32_t write_hit_l2;
extern uint32_t write_unhit_l2;
extern uint32_t read_hit_l2;
extern uint32_t read_unhit_l2;

uint64_t cycle;
uint32_t hit;
uint32_t unhit;
uint32_t write_hit;
uint32_t write_unhit;
uint32_t read_hit;
uint32_t read_unhit;
uint64_t cycle_l2;
uint32_t hit_l2;
uint32_t unhit_l2;
uint32_t write_hit;
uint32_t write_unhit;
uint32_t read_hit;
uint32_t read_unhit;

typedef struct Block{
    uint8_t space[BLOCK_SIZE];
    uint32_t tag;
    bool valid;
    bool dirty;
}Block;

struct Cache;

typedef struct Cache {
    Block *block;
    uint32_t set_way;
    uint32_t cache_size;
    void (*init)(struct Cache *self);
    uint32_t (*replace_algo)(struct Cache *self,uint32_t addr);
    uint32_t (*map_algo)(struct Cache *self,swaddr_t addr);
    uint8_t (*cache_read)(struct Cache *self,uint32_t bid,uint32_t offset);
    uint32_t (*read)(struct Cache *self,uint32_t addr,uint32_t len);
    void (*write_defect)(struct Cache *self,swaddr_t addr,void* data,uint32_t len);
    void (*cache_write)(struct Cache *self,uint32_t bid,uint8_t data,uint32_t offset);
    void (*write)(struct Cache *self,uint32_t addr,void* data,uint32_t len);

}Cache;


void init_default(Cache *self);

uint32_t read_default(Cache *self,uint32_t addr,uint32_t len);

uint8_t cache_read_default(Cache *self,uint32_t bid,uint32_t offset);

uint32_t group_rand_replace(Cache *self,swaddr_t addr);

uint32_t group_mapping(Cache *self,swaddr_t addr);

void cache_write_default(Cache *self,uint32_t bid,uint8_t data,uint32_t offset);

void write_default(Cache *self,uint32_t addr,void* data ,uint32_t len);

void write_defect_straight(Cache *self,swaddr_t addr, void* data,uint32_t len);


#endif