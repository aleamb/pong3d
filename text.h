#ifndef _TEXT_H_
#define _TEXT_H_

int init_text_renderer();
void render_text(const char* text, float x, float y, float scale, int font_size);
void dispose_text_renderer();

#endif
