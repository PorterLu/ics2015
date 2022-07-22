#include "common.h"

#ifdef HAS_DEVICE

#include "sdl.h"
#include "vga.h"

#include <sys/time.h>
#include <time.h>
#include <signal.h>

SDL_Surface *real_screen;
SDL_Surface *screen;
uint8_t (*pixel_buf) [SCREEN_COL];

#define TIMER_HZ 100

static uint64_t jiffy = 0;
static struct itimerval it;
static int device_update_flag = false;
static int update_screen_flag = false;
extern void timer_intr();
extern void keyboard_intr();
extern void update_screen();
static int sdl_event_pending = 0;

//时钟频率为100hz，每当一个时钟到来的时候，执行时钟中断函数，并且根据时钟是否%4余0，设置屏幕更行位，最后再启用一个时钟
static void timer_sig_handler(int signum) {
	jiffy ++;
	timer_intr();

	device_update_flag = true;
	if(jiffy % (TIMER_HZ / VGA_HZ) == 0) {
		update_screen_flag = true;
	}

	int ret = setitimer(ITIMER_VIRTUAL, &it, NULL);
	Assert(ret == 0, "Can not set timer");
}


//device_update 根据屏幕更行位判断是否需要跟新屏幕，同时动作池中取出键盘的行为，并出发中断
void device_update() {
	if(!device_update_flag) {
		return;
	}
	device_update_flag = false;

	if(update_screen_flag) {
		update_screen();
		update_screen_flag = false;
	}

	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		// If a key was pressed

		uint32_t sym = event.key.keysym.sym;
		if( event.type == SDL_KEYDOWN ) {
			keyboard_intr(sym2scancode[sym >> 8][sym & 0xff]);
		}
		else if( event.type == SDL_KEYUP ) {
			keyboard_intr(sym2scancode[sym >> 8][sym & 0xff] | 0x80);
		}

		// If the user has Xed out the window
		if( event.type == SDL_QUIT ) {
			//Quit the program
			exit(0);
		}
	}
}


//计算两个时间相差的毫秒数
static double ts_minus(const struct timespec *tsp1, const struct timespec *tsp2)
{
    /* tsp1 - tsp2 (in ms)
     * note: pay attation to rounding method */
    return (tsp1->tv_sec - tsp2->tv_sec) * 1000 +
           (tsp1->tv_nsec - tsp2->tv_nsec) / 1000000.0;
}


static struct timespec last_device_update_ts;
double tot_timer_jitter = 0;
double tot_timer_cnt = 0;
#define TS_DIFF (1000 / TIMER_HZ)
//每次检查都查看距离上一次执行这个函数并update是否已经过了10ms，是的话执行devide_update()
void check_device_update()
{
    static double diff_target;
    static int init_flag = 0;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    if (!init_flag) { // should init
        last_device_update_ts = ts;
        diff_target = TS_DIFF;
        init_flag = 1;
        return;
    }

    double diff = ts_minus(&ts, &last_device_update_ts);
    if (sdl_event_pending) {
        device_update();
    } else if (diff >= diff_target) { //时间是否已经过了一个10ms
        double jitter = (diff - diff_target);
        diff_target = TS_DIFF - jitter;
        tot_timer_jitter += jitter;
        tot_timer_cnt++;
        last_device_update_ts = ts;
        device_update();
	}
} 


void sdl_clear_event_queue() {
	SDL_Event event;
	while(SDL_PollEvent(&event));
}

void init_sdl() {
	int ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);
	Assert(ret == 0, "SDL_Init failed");

	real_screen = SDL_SetVideoMode(640, 400, 8, 
			SDL_HWSURFACE | SDL_HWPALETTE | SDL_HWACCEL | SDL_ASYNCBLIT);

	screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 400, 8,
			real_screen->format->Rmask, real_screen->format->Gmask,
			real_screen->format->Bmask, real_screen->format->Amask);
	pixel_buf = screen->pixels;

	SDL_SetPalette(real_screen, SDL_LOGPAL | SDL_PHYSPAL, (void *)&palette, 0, 256);
	SDL_SetPalette(screen, SDL_LOGPAL, (void *)&palette, 0, 256);

	SDL_WM_SetCaption("NEMU", NULL);

	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	struct sigaction s;
	memset(&s, 0, sizeof(s));
	s.sa_handler = timer_sig_handler;
	ret = sigaction(SIGVTALRM, &s, NULL);
	Assert(ret == 0, "Can not set signal handler");

	it.it_value.tv_sec = 0;
	it.it_value.tv_usec = 1000000 / TIMER_HZ;
	ret = setitimer(ITIMER_VIRTUAL, &it, NULL);
	Assert(ret == 0, "Can not set timer");
}
#endif	/* HAS_DEVICE */
