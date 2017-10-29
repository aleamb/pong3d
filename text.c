/**

  @file text.c
  @author Alejandro Ambroa
  @date 1 Oct 2017
  @brief Rendering text using freetype lib. 

 */

#include "text.h"
#include "geometry.h"
#include <stdio.h>
#include "renderer.h"
#include <ft2build.h>
#include FT_FREETYPE_H

GLuint renderTextUniform;
GLuint modelMatrixUniform;

FT_Library ft;
FT_Face face;

int freetype_initialized = 0;

static float text_model_matrix[16];

void render_text(const char* text, float x, float y, float scale, int font_size) {

	GLuint vao, vbo, texture;

	const char *p;

	text_model_matrix[0] = scale;
	text_model_matrix[5] = -scale;
	text_model_matrix[10] = scale;
	text_model_matrix[12] = x - scale * (strlen(text) >> 1);
	text_model_matrix[13] = y;
	float dx = 0.0f;

	glUniform1i(renderTextUniform, 1);
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenTextures(1, &texture);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (char*)NULL + (sizeof(float) * 4));
	glEnableVertexAttribArray(3);
	glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, text_model_matrix);
	FT_Set_Pixel_Sizes(face, 0, font_size);
	for(p = text; *p; p++) {
		if(FT_Load_Char(face, *p, FT_LOAD_RENDER))
			continue;
		FT_GlyphSlot g = face->glyph;

		GLfloat box[] = {
			1.0 + dx, 1.0, 0.2, 1.0, 1, 1,
			1.0 + dx, 0.0, 0.2, 1.0, 1, 0,
			0   + dx, 1.0, 0.2, 1.0, 0, 1,
			0   + dx, 0,   0.2, 1.0, 0, 0};

		dx += 1.0;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				g->bitmap.width,
				g->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				g->bitmap.buffer
			    );

		glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	glBindVertexArray(0);
	glUniform1i(renderTextUniform, 0);

	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	glDeleteTextures(1, &texture);
}


int init_text_renderer() {
	if(FT_Init_FreeType(&ft)) {
		fprintf(stderr, "Could not init freetype library\n");
		return -1;
	}
	freetype_initialized = 1;
	if(FT_New_Face(ft, "./fonts/main.ttf", 0, &face)) {
		fprintf(stderr, "Could not open font\n");
		return -1;
	}
	GLuint program = renderer_get_main_program();
	glUniform1i(glGetUniformLocation(program, "tex"), 0);
	renderTextUniform = glGetUniformLocation(program, "renderText");
	modelMatrixUniform = glGetUniformLocation(program, "modelMatrix"); 
	// generate start screen text
	load_identity_matrix(text_model_matrix);
	return 0;
}

void dispose_text_renderer() {
	if (freetype_initialized) {
		FT_Done_FreeType(ft);
	}
}

