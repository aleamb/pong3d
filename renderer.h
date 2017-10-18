#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "geometry.h"

int init_renderer(int width, int height);
GLuint renderer_get_main_program();
GLuint build_shaders_program(int count, int* result, GLchar* errormsg, ...) ;
void render_pong_element(PONG_ELEMENT* element); 
void upload_to_renderer(PONG_ELEMENT*);
void remove_to_renderer(PONG_ELEMENT*);
void dispose_renderer();

void render_stage();
void render_sticks();
void render_ball();
void render_balls_counter(int);

#endif
