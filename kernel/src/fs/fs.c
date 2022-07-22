#include "common.h"
#include <string.h>

typedef struct {
    bool opened;
    uint32_t offset;
} Fstate;

typedef struct {
	char *name;
	uint32_t size;
	uint32_t disk_offset;
} file_info;

enum {SEEK_SET, SEEK_CUR, SEEK_END};

/* This is the information about all files in disk. */
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

Fstate fstate[ NR_FILES + 3 ];

int fs_open(const char *pathname, int flags);    /* 在我们的实现中可以忽略flags */
int fs_read(int fd, void *buf, uint32_t len);
int fs_write(int fd, void *buf, uint32_t len);
int fs_lseek(int fd, int offset, int whence);
int fs_close(int fd);
void ide_read(uint8_t *, uint32_t, uint32_t);
void ide_write(uint8_t *, uint32_t, uint32_t);

/* TODO: implement a simplified file system here. */

int fs_open(const char *pathname, int flags)
{
	int i;
	//printk("syscall open: pathname: %s  flags:%d\n",pathname, flags);

	for( i = 0; i < NR_FILES; i++ )
	{
		if( strcmp( file_table[i].name, pathname) == 0 )
		{
			fstate[ i + 3 ].opened = true;
			fstate[ i + 3 ].offset = 0 ;
			break;
		}
	}
	return i + 3;
}

int fs_read( int fd, void *buf, uint32_t len )
{
	if( fstate[ fd ].opened == false )
		return -1;
	//printk("syscall read: fd:%d  len:%d\n", fd, len);

	int disk_offset = file_table[ fd - 3 ].disk_offset;
	int offset = fstate[ fd ].offset;
	if( offset + len > file_table[ fd - 3 ].size )
		len = file_table[ fd - 3 ].size - offset;
	ide_read( (uint8_t*)buf, disk_offset + offset, len );
	fstate[ fd ].offset += len;

	return len;
}

int fs_write( int fd, void *buf, uint32_t len )
{
	if( fstate[ fd ].opened == false &&  fd > 2 )
		return -1;
	//printk("syscall write: fd:%d  len:%d\n", fd, len);

	int disk_offset = file_table[ fd - 3 ].disk_offset;
	int offset = fstate[ fd ].offset;
	if( offset + len > file_table[ fd - 3 ].size )
		len = file_table[ fd - 3 ].size - offset;
	ide_write( (uint8_t*)buf, disk_offset + offset, len );
	fstate[ fd ].offset += len;

	return len; 
}

int fs_lseek( int fd, int offset, int whence )
{
	if( fstate[ fd ].opened == false )
	{
	//	printk("%s (fd:%d) unopened\n",file_table[ fd - 3 ].name, fd);
		return -1;
	}

	//printk("syscall fs_lseek: fd:%d  offset:%d whence:%d\n", fd, offset, whence);
	switch( whence )
	{
		case SEEK_SET:
			if( offset >= 0 && offset <= file_table[ fd + 3 ].size )
				fstate[ fd ].offset = offset;
			else
				return -1;
			break;
		case SEEK_CUR:
			if( fstate[ fd ].offset + offset >= 0 && fstate[ fd ].offset + offset <= file_table[ fd + 3 ].size)
				fstate[ fd ].offset += offset;
			else
				return -1;
			break;
		case SEEK_END:
			if( file_table[ fd - 3 ].size + offset >= 0 && offset <= 0 )
				fstate[ fd ].offset = file_table[ fd - 3 ].size + offset ;
			else
				return -1;
	}

	return fstate[ fd ].offset;
}

int fs_close( int fd )
{
	//printk("syscall fs_close: fd:%d\n", fd);

	fstate[ fd ].opened = false;
	return 0;
}

