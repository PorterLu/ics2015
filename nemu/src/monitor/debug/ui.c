#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"
#include "memory/cache.h"
#include "cpu/reg.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

//#define recursion_depth 100
Cache* cache;

void cpu_exec(uint32_t);
char* findFunc(uint32_t addr);


static int cmd_help(char *args);
static int cmd_c(char *args);
static int cmd_q(char* args);
static int cmd_si(char* args);
static int cmd_info(char* args);
static int cmd_p(char* args);
static int cmd_x(char* args);
static int cmd_w(char* args);
static int cmd_d(char* args);
static int cmd_bt(char* args);
static int cmd_cache(char* args);
static int cmd_gdtr(char* args);

static struct {
        char *name;
        char *description;
        int (*handler) (char *);
} cmd_table [] = {
        { "help", "Display informations about all supported commands", cmd_help },
        { "c", "Continue the execution of the program", cmd_c },
        { "q", "Exit NEMU", cmd_q },
        { "si", "single instruction", cmd_si},
        { "info", "print program state", cmd_info},
        { "p", "calculate expression value",cmd_p},
        { "x", "memory scan",cmd_x},
        { "w", "set watchpoint", cmd_w},
        { "d", "delete watchpoint",cmd_d},
        { "bt", "backtrace",cmd_bt},
		{ "cache","show the cache content",cmd_cache},
		{ "gdtr","show the information of gdtr",cmd_gdtr}
        /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

/* We use the ``readline'' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

int cmd_q(char *args) {
	return -1;
}


int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}


int cmd_si(char *args)
{
	char *arg = strtok(NULL," ");
	if(arg == NULL)
		cpu_exec(1);
	else
	{
		int tmp;
		sscanf(arg,"%d",&tmp);
		if(tmp > 0)
			cpu_exec(tmp);		
	}
	return 0;
}


int cmd_info(char *args)
{
	args = strtok(NULL," ");
	if(!strcmp(args,"w"))
	{
		print_wp();
	}				
	else if(!strcmp(args,"r"))
	{
		printf("register list:\n");
		printf("eax:%x  ebx:%x  ecx:%x  edx:%x\n",cpu.eax,cpu.ebx,cpu.ecx,cpu.edx);
		printf("esi:%x  edi:%x  ebp:%x  esp:%x  eip:%x\n",cpu.esi,cpu.edi,cpu.ebp,cpu.esp,cpu.eip);
		printf("es:%x  cs:%x  ss:%x  ds:%x  fs:%x  gs:%x\n",cpu.es.sreg,cpu.cs.sreg,cpu.ss.sreg,cpu.ds.sreg,cpu.fs.sreg,cpu.gs.sreg);
		printf("gdtr.base:%x  gdtr.limit:%x\n",cpu.gdtr.base,cpu.gdtr.limit);
		printf("idtr.base:%x  idtr.limit:%x\n",cpu.idtr.base,cpu.idtr.limit);
		printf("cr0:%x  cr3:%x\n",cpu.cr0.val,cpu.cr3.val);

	}
	else if(!strcmp(args,"e"))
	{
		printf("eflags:\n");
		printf("CF:%d  OF:%d  SF:%d  ZF:%d  PF:%d  DF:%d  IF:%d\n",eflags.CF,eflags.OF,eflags.SF,eflags.ZF,eflags.PF,eflags.DF,eflags.IF);
	}
	return 0;

}

int cmd_p(char *args)
{
	bool success=true;
	char *str = strtok(NULL," ");
	uint32_t value = expr(str,&success);
	printf("0x%x\n",value);
	return 0;
}

int cmd_x(char *args)
{	int n,addr_n,i;
	bool success = true;
	char *num = strtok(NULL," ");
	char *addr = strtok(NULL," ");
	sscanf(num,"%d",&n);
	addr_n = expr(addr,&success);
	for(i=0;i<n*4;i++)
	{
		if(i%4 == 0)
			printf("%08x: ",addr_n+i);
		printf("%02x ",swaddr_read(addr_n+i,1,DS));
		if(i%4 == 3)
			printf("\n");
	}
	return 0;
}

int cmd_w(char *args)
{
	bool success = true;
	char *expression = strtok(NULL," ");
	WP *wp =new_wp();
	strcpy(wp->expr,expression); 
	wp->value = expr(expression,&success);
	return 0;	
}

int cmd_d(char *args)
{
	int no;
	char *str = strtok(NULL," ");
	sscanf(str,"%d",&no);
	free_wp(no);
	return 0;
}

/*
typedef struct {
    swaddr_t prev_ebp;
    swaddr_t ret_addr;
    uint32_t args[4];
} PartOfStackFrame;
*/

int cmd_bt(char *args)
{
	//PartOfStackFrame frame[recursion_depth];
	//uint32_t index=0;
	uint32_t ebp = cpu.ebp;
	char *func_name = NULL;
	uint32_t  now_addr = cpu.eip;


	while(ebp!=0) 
	{
		//printf("%d\n",*(uint32_t*)(ebp+hw_mem));
		printf("ebp: %08x ,pre_ebp:%08x ",ebp,swaddr_read(ebp,4,SS)); 
		printf("args: %08x %08x %08x %08x \n",swaddr_read(ebp + 8,4,SS),\
											swaddr_read(ebp + 12,4,SS),\
											swaddr_read(ebp + 16,4,SS),\
											swaddr_read(ebp + 20,4,SS));
		func_name = findFunc(now_addr);
		if(func_name != NULL)
			printf("return_arr: %08x ,now in %s() \n\n",swaddr_read(ebp + 4,4,SS),func_name);
		//now_addr = *((uint32_t*)(hw_mem + ebp + 4));
		now_addr = swaddr_read(ebp+4,4,SS);
		ebp = swaddr_read(ebp,4,SS);
	} 
	return 0;

}

int cmd_cache(char *args)
{
	char *str = strtok(NULL," ");
	uint32_t addr;
	bool success = true;
	addr = expr(str,&success);
	uint32_t index = cache->map_algo(cache,addr);
	if(index !=-1 && cache->block[index].tag == (addr/BLOCK_SIZE)/((CACHE_SIZE/BLOCK_SIZE)/cache->set_way))
	{
		uint32_t i;
		printf("Cache contents:\n");
		for(i=0;i<BLOCK_SIZE;i++)
		{
			printf("%02x ",cache->block[index].space[i]);
			if(i%16 == 15)
				printf("\n");
		}
		printf("\n");
	}
	else
	{
		printf("data is not in cache\n");
	}
	return 0;
}

int cmd_gdtr(char *args)
{
	uint32_t gdtr_base = cpu.gdtr.base;
	int num = (cpu.gdtr.limit + 1)/8,i = 0;
	for(i=0;i<num;i++)
	{
		uint32_t temp,base,limit;
		temp = lnaddr_read(gdtr_base + i*8 + 4,4);
		base = (temp&0xff000000) | ((temp&0xff)<<16);
		limit = temp&0x000f0000;
		temp = lnaddr_read(gdtr_base + i*8 ,4);
		base = ((temp&0xffff0000) >>16)|base;
		limit = (temp&0xffff)|limit;
		printf("NO:%d  base:%x  limit:%x\n",i,base,limit);
	}

	for(i=0;i< SR_NUM;i++)
	{
		uint32_t temp;
		uint32_t base,limit;

		switch(i)
		{
			case 0: printf("es: ");break;
			case 1: printf("cs: ");break;
			case 2: printf("ss: ");break;
			case 3: printf("ds: ");break;
			case 4: printf("fs: ");break;
			case 5: printf("gs: ");
		}

		printf("No:%d ",temp = cpu.sr[i].sreg/8);
		//printf("%016lx\n",cpu.sr[i].gdesc);
		temp = (uint32_t)((cpu.sr[i].gdesc&0xffffffff00000000)>>32);
		base = (temp&0xff000000) | ((temp&0xff)<<16);
		limit = temp&0x000f0000;
		temp = (uint32_t)cpu.sr[i].gdesc;
		base = ((temp&0xffff0000) >>16)|base;
		limit = (temp&0xffff)|limit;
		printf("base:%x  limit:%x\n",base,limit);
	}

	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
