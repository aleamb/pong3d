/**
	@file text.h
	@author Alejandro Ambroa
	@date 1 Oct 2017
	@brief Definitions for text rendering.
*/

#ifndef _TEXT_H_
#define _TEXT_H_

int init_text_renderer();
void render_text(const char* text, float x, float y, float scale, int font_size);
void dispose_text_renderer();

#endif
