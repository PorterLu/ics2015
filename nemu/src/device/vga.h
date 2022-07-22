#ifndef __VGA_H__
#define __VGA_H__

#include "common.h"
#include <SDL/SDL.h>

#define SCREEN_ROW 400
#define SCREEN_COL 640
#define VGA_HZ 25

extern SDL_Surface *real_screen;
extern SDL_Surface *screen;

//this is array that basic type is 400 uint8_t .
extern uint8_t (*pixel_buf) [SCREEN_COL];

static inline void draw_pixel(int x, int y, uint8_t color_idx) {
	assert(x >= 0 && x < SCREEN_COL && y >= 0 && y < SCREEN_ROW);
	pixel_buf[y][x] = color_idx; // the index to render the pixel
}

typedef union {
	uint32_t val;
	struct { 
		uint8_t r, g, b, a;
	};
} Color;

extern Color palette[];

#endif
