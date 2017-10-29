/**
	@file geometry.h
	@author Alejandro Ambroa
	@date 1 Oct 2017
	@brief System functions definitions. 
*/

#ifndef _MSYS_H_
#define _MSYS_H_

typedef enum {
	MOUSEMOTION,
	MOUSELBUTTONUP,
	CLOSE
}SysEventType;

typedef struct {
	SysEventType type;
	int x;
	int y;
	int prevx;
	int prevy;
}SysEvent;

int sys_init_video(int width, int height);
int sys_init_sound(int sample_rate);
void sys_play_sound(void *samples, int data_size);
void sys_dispose_video();
void sys_dispose_audio();
void sys_quit();
unsigned int sys_get_ticks();
int sys_wait(SysEvent* event, unsigned int milis);

void sys_swap_buffers();
void sys_mouse_center(int width, int height);
void sys_show_cursor(int show); 
#endif
