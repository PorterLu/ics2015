#include "irq.h"
#include <syscall.h>
#define SERIAL_PORT  0x3F8

typedef struct {
    bool opened;
    uint32_t offset;
} Fstate;

typedef struct {
	char *name;
	uint32_t size;
	uint32_t disk_offset;
} file_info;

static const file_info file_table[] __attribute__((used)) = {
	{"1.rpg", 188864, 1048576}, {"2.rpg", 188864, 1237440},
	{"3.rpg", 188864, 1426304}, {"4.rpg", 188864, 1615168},
	{"5.rpg", 188864, 1804032}, {"abc.mkf", 1022564, 1992896},
	{"ball.mkf", 134704, 3015460}, {"data.mkf", 66418, 3150164},
	{"desc.dat", 16027, 3216582}, {"fbp.mkf", 1128064, 3232609},
	{"fire.mkf", 834728, 4360673}, {"f.mkf", 186966, 5195401},
	{"gop.mkf", 11530322, 5382367}, {"map.mkf", 1496578, 16912689},
	{"mgo.mkf", 1577442, 18409267}, {"m.msg", 188232, 19986709},
	{"mus.mkf", 331284, 20174941}, {"pat.mkf", 8488, 20506225},
	{"rgm.mkf", 453202, 20514713}, {"rng.mkf", 4546074, 20967915},
	{"sss.mkf", 557004, 25513989}, {"voc.mkf", 1997044, 26070993},
	{"wor16.asc", 5374, 28068037}, {"wor16.fon", 82306, 28073411},
	{"word.dat", 5650, 28155717},
};
#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

extern Fstate fstate[ NR_FILES + 3 ];

int fs_open(const char *pathname, int flags);    /* 在我们的实现中可以忽略flags */
int fs_read(int fd, void *buf, int len);
int fs_write(int fd, void *buf, int len);
int fs_lseek(int fd, int offset, int whence);
int fs_close(int fd);
void add_irq_handle(int, void (*)(void));
void mm_brk(uint32_t);

static void sys_brk(TrapFrame *tf) {
#ifdef IA32_PAGE
	mm_brk(tf->ebx);
#endif
	tf->eax = 0;
}

void do_syscall(TrapFrame *tf) {
	switch(tf->eax) {
		/* The ``add_irq_handle'' system call is artificial. We use it to 
		 * let user program register its interrupt handlers. But this is 
		 * very dangerous in a real operating system. Therefore such a 
		 * system call never exists in GNU/Linux.
		 */
		case 0: 
			cli();
			add_irq_handle(tf->ebx, (void*)tf->ecx);
			sti();
			break;

		case SYS_brk: sys_brk(tf); break;
		case SYS_read: //read
			//fd, buf, len
			{
				int fd = tf->ebx;
				char *buf = (void*) tf->ecx;
				int len = tf->edx;
				printk("in do_syscall_read fd:%d, len:%d at %d\n",fd,tf->edx,file_table[fd-3].disk_offset+fstate[fd].offset);

				int return_value = fs_read( fd, buf, len);
				tf->eax = return_value;
			}
			break;
		/* TODO: Add more system calls. */
		case SYS_write:
 				if (tf->ebx == 1 || tf->ebx == 2) {
                        // get paramaters
                    char *buf = (void *) tf->ecx;
                    int len = tf->edx,i;

                        // do real write
                    for(i=0;i<len;i++)
						out_byte(SERIAL_PORT,*(buf + i));

                        // set return value (number of bytes written)
                    tf->eax = len;
				}
				else if(tf->ebx > 3)
				{
					int fd = tf->ebx;
					char *buf = (void*) tf->ecx;
					int len = tf->edx;
					printk("in do_syscall_write fd:%d, len:%d at %d\n",fd, len,file_table[fd-3].disk_offset+fstate[fd].offset);

					int return_value = fs_write( fd, buf, len );
					tf->eax = return_value;

				}
			break;
			//else 
			//{
            //    tf->eax = fs_write(tf->ebx, (void *) tf->ecx, tf->edx);
            //}

		case SYS_open:
			{
				int flag = tf->ecx;
				char* filename = (void*) tf->ebx;
				printk("in do_syscall_open fd:%s\n",filename);

				int fd = fs_open( filename, flag);

				tf->eax = fd;
				
			}
			break;
		case SYS_close:
			{
				int fd =  tf->ebx;
				int return_value = fs_close( fd );
				printk("in do_syscall_close fd:%d\n",fd);

				tf->eax = return_value;
			}	
			break;
		case SYS_lseek:
			{
				int fd = tf->ebx;
				int offset = tf->ecx;
				int whence = tf->edx;
				printk("in do_syscall_lseek fd:%d, offset:%d, whence:%d\n",fd,offset,whence);
				int return_value = fs_lseek( fd, offset, tf->edx );
				tf->eax = return_value;
			}
			break;
		default: panic("Unhandled system call: id = %d", tf->eax);
	}
}

