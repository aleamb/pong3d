#ifndef _MSYS_H_
#define _MSYS_H_

int sys_init_screen(int width, int height);
int sys_init_sound(int sample_rate);
void sys_play_sound(void *samples, int data_size);
void sys_dispose_screen();
void sys_dispose_audio();
void sys_quit();

#endif
