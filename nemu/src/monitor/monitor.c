#include "nemu.h"
#include "memory/cache.h"
#include <assert.h>

#define ENTRY_START 0x100000
#define EFLAGS_VALUE 0x00

extern uint8_t entry [];
extern uint32_t entry_len;
extern char *exec_file;

void load_elf_tables(int, char *[]);
void init_regex();
void init_wp_pool();
void init_ddr3();
void init_cache();
void init_L2();
void init_device();
void init_sdl();

FILE *log_fp = NULL;

static void init_log() {
	log_fp = fopen("log.txt", "w");
	Assert(log_fp, "Can not open 'log.txt'");
}

static void welcome() {
	printf("Welcome to NEMU!\nThe executable is %s.\nFor help, type \"help\"\n",
			exec_file);
}

void init_monitor(int argc, char *argv[]) {
	/* Perform some global initialization */

	/* Open the log file. */
	init_log();

	/* Load the string table and symbol table from the ELF file for future use. */
	load_elf_tables(argc, argv);

	/* Compile the regular expressions. */
	init_regex();

	/* Initialize the watchpoint pool. */
	init_wp_pool();

	/* Display welcome message. */
	welcome();
}

#ifdef USE_RAMDISK
static void init_ramdisk() {
	int ret;
	const int ramdisk_max_size = 0xa0000;
	FILE *fp = fopen(exec_file, "rb");
	Assert(fp, "Can not open '%s'", exec_file);

	fseek(fp, 0, SEEK_END);
	size_t file_size = ftell(fp);
	Assert(file_size < ramdisk_max_size, "file size(%zd) too large", file_size);

	fseek(fp, 0, SEEK_SET);
	ret = fread(hwa_to_va(0), file_size, 1, fp);
	assert(ret == 1);
	fclose(fp);
}
#endif

static void load_entry() {
	int ret;
	FILE *fp = fopen("entry", "rb");
	Assert(fp, "Can not open 'entry'");

	fseek(fp, 0, SEEK_END);
	size_t file_size = ftell(fp);

	fseek(fp, 0, SEEK_SET);
	ret = fread(hwa_to_va(ENTRY_START), file_size, 1, fp);
	assert(ret == 1);
	fclose(fp);
}


void restart() {
	/* Perform some initialization to restart a program */
#ifdef USE_RAMDISK
	/* Read the file with name `argv[1]' into ramdisk. */
	init_ramdisk();
#endif

	/* Read the entry code into memory. */
	load_entry();

	/* Set the initial instruction pointer. */
	cpu.eip =  ENTRY_START;

	/* Set the initial value of EFLAGS*/
	eflags.val = EFLAGS_VALUE;

	/* Initialize DRAM. */
	init_ddr3();

	init_cache();

	init_L2();

	uint64_t base=0, limit=0xffffffff;
	cpu.sr[CS].sreg = 0x08;
	cpu.sr[CS].gdesc = (((limit)>>12 & 0xffff ) | ((base)&0xffff)<<16 )| (((base)>>16 & 0xff) << 32) | \
	(((uint64_t)(uint8_t)0x90 | 0xA)<<40)|(((0xC0| (limit >> 28)) & 0xff) << 48)| (((base >> 24)&0xff)<<54);  
	//printf("%lx\n",cpu.sr[CS].gdesc);

	cpu.cr0.protect_enable = 0;
	cpu.cr0.paging = 0;

	init_device();

	init_sdl();

}
