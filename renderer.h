#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "geometry.h"

int init_renderer(int width, int height);
GLuint renderer_get_main_program();
GLuint build_shaders_program(int count, int* result, GLchar* errormsg, ...);
void render_pong_element(PONG_ELEMENT* element);
void upload_to_renderer(PONG_ELEMENT*);
void remove_to_renderer(PONG_ELEMENT*);
void dispose_renderer();

void render_stage();
void render_ball();
void render_balls_counter(int);

void render_fadeout_overlay(float overlay_fadeout_alpha);
void renderer_clear_screen();
void render_overlay();
void reset_overlay();

void render_opponent_stick();
void render_player_stick();
void render_balls_counter(int);
void render_shadows();

#endif
