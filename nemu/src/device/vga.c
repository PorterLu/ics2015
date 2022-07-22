#include "common.h"

#ifdef HAS_DEVICE

#include "vga.h"
#include "device/port-io.h"
#include "device/mmio.h"
#include "device/i8259.h"


//this is the enum type used to describe vga register
enum {Horizontal_Total_Register, End_Horizontal_Display_Register, 
	Start_Horizontal_Blanking_Register, End_Horizontal_Blanking_Register,
   	Start_Horizontal_Retrace_Register, End_Horizontal_Retrace_Register,
   	Vertical_Total_Register, Overflow_Register, Preset_Row_Scan_Register,
   	Maximum_Scan_Line_Register, Cursor_Start_Register, Cursor_End_Register,
   	Start_Address_High_Register, Start_Address_Low_Register, Cursor_Location_High_Register,
   	Cursor_Location_Low_Register, Vertical_Retrace_Start_Register,
   	Vertical_Retrace_End_Register, Vertical_Display_End_Register, Offset_Register,
   	Underline_Location_Register, Start_Vertical_Blanking_Register, End_Vertical_Blanking,
   	CRTC_Mode_Control_Register, Line_Compare_Register
};

static uint8_t *vga_dac_port_base;
static uint8_t *vga_crtc_port_base;
static uint8_t vga_crtc_regs[19];

#define VGA_DAC_READ_INDEX 0x3C7
#define VGA_DAC_WRITE_INDEX 0x3C8
#define VGA_DAC_DATA 0x3C9

#define VGA_CRTC_INDEX		0x3D4
#define VGA_CRTC_DATA		0x3D5

#define CTR_ROW 200
#define CTR_COL 320

static void *vmem_base;
bool vmem_dirty = false;
bool line_dirty[CTR_ROW];

//每次in/out显存时进行的回调函数，只有是out时这个执行才有真正执行，设置写所在的行的dirty为1
void vga_vmem_io_handler(hwaddr_t addr, size_t len, bool is_write) {
	if(is_write) {
		int line = (addr - 0xa0000) / CTR_COL;
		if(line < CTR_ROW) {
			line_dirty[line] = true;
			vmem_dirty = true;
		}
	}
}

/**
 * @brief vmem 是一个以行列为基准的矩阵，pixel_buf中是SDL中的x，y坐标为基准的一个矩阵
 * 对vmem做一个行列的扫描，由于vmem中的一个像素对应screen中的4个像素，vmem中一个点要更新4次
 * 行对应y轴，列对应x轴，最后再改回行列的模式更改pixel_buf中的值，之后将修改的部分映射到物理屏幕上。
 * 
 */

void do_update_screen_graphic_mode() {
	int i, j;
	uint8_t (*vmem) [CTR_COL] = vmem_base;
	SDL_Rect rect;
	rect.x = 0;
	rect.w = CTR_COL * 2;
	rect.h = 2;

	for(i = 0; i < CTR_ROW; i ++) {
		if(line_dirty[i]) {
			for(j = 0; j < CTR_COL; j ++) {
				uint8_t color_idx = vmem[i][j];
				draw_pixel(2 * j, 2 * i, color_idx);
				draw_pixel(2 * j, 2 * i + 1, color_idx);
				draw_pixel(2 * j + 1, 2 * i, color_idx);
				draw_pixel(2 * j + 1, 2 * i + 1, color_idx);
			}
			rect.y = i * 2;

			//map logical screen on physic screen 
			SDL_BlitSurface(screen, &rect, real_screen, &rect);
		}
	}
	SDL_Flip(real_screen);
}

void update_screen() {
	if(vmem_dirty) {
		do_update_screen_graphic_mode();
		vmem_dirty = false;
		memset(line_dirty, false, CTR_ROW);
	}
}


/* palette contains 256 color used to show on screen, vga_dac_port figure out the index of it*/
/*
这个函数用于修改调色板，操作时先设置要修改的调色板的坐标，如果修改的是四个颜色域中的最后一个，判断是否已经是调色板的最后一个index
最后设置调色板到逻辑屏幕和物理屏幕上。
*/

void vga_dac_io_handler(ioaddr_t addr, size_t len, bool is_write) {
	static uint8_t *color_ptr; 
	if(addr == VGA_DAC_WRITE_INDEX && is_write) {
		color_ptr = (void *)&palette[ vga_dac_port_base[0] ];
	}
	else if(addr == VGA_DAC_DATA && is_write) {
		*color_ptr++ = vga_dac_port_base[1] << 2;
		if( (((void *)color_ptr - (void *)&screen->format->palette->colors) & 0x3) == 3) {
			color_ptr ++;
			if((void *)color_ptr == (void *)&palette[256]) {
				SDL_SetPalette(real_screen, SDL_LOGPAL | SDL_PHYSPAL, (void *)&palette, 0, 256);
				SDL_SetPalette(screen, SDL_LOGPAL, (void *)&palette, 0, 256);
			}
		}
	}
}


/*如何操作控制寄存器？1.向VGA_CRTC_INDEX中写入control register的编号，VGA_CTRL_DATA中就可以获取该control寄存器的值；
2.向VGA_CTRL_DATA中写入数据，根据VGA_CRTL_INDEX的值，将对应的control register设为输入的数据
*/
void vga_crtc_io_handler(ioaddr_t addr, size_t len, bool is_write) {
	if(addr == VGA_CRTC_INDEX && is_write) {
		vga_crtc_port_base[1] = vga_crtc_regs[ vga_crtc_port_base[0] ];
	}
	else if(addr == VGA_CRTC_DATA && is_write) {
		vga_crtc_regs[ vga_crtc_port_base[0] ] = vga_crtc_port_base[1] ;
	}
}



//this function is used to initilize the i/o port and memory to work with vga( control register and data register)
void init_vga() {
	vga_dac_port_base = add_pio_map(VGA_DAC_WRITE_INDEX, 2, vga_dac_io_handler);
	vga_crtc_port_base = add_pio_map(VGA_CRTC_INDEX, 2, vga_crtc_io_handler);
	vmem_base = add_mmio_map(0xa0000, 0x20000, vga_vmem_io_handler);
}
#endif	/* HAS_DEVICE */
