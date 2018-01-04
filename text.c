/**

  @file text.c
  @author Alejandro Ambroa
  @date 1 Oct 2017
  @brief Rendering text using freetype lib. 

 */

#include "text.h"
#include "geometry.h"
#include "pong3d.h"
#include "renderer.h"
#include "msys.h"
#include <ft2build.h>
#include <stdio.h>
#include FT_FREETYPE_H

#define NUM_LETTERS 96
GLuint renderTextUniform;
GLuint modelMatrixUniform;

FT_Library ft;
FT_Face face;

int freetype_initialized = 0;

static float text_model_matrix[16];
static GLuint vao, vbo;

static GLfloat box[] = {
    1.0f, 1.0f, 0.2f, 1.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 0.2f, 1.0f, 1.0f, 0,
    0, 1.0f, 0.2f, 1.0f, 0, 1.0f,
    0, 0, 0.2f, 1.0f, 0, 0
};

static GLuint textures[NUM_LETTERS] = { 0 };

void render_text(const char* text, float x, float y, float scale)
{

    const char* p;

    text_model_matrix[0] = scale;
    text_model_matrix[5] = -scale;
    text_model_matrix[10] = scale;
    text_model_matrix[12] = x - scale * (strlen(text) >> 1);
    text_model_matrix[13] = y;

    glUniform1i(renderTextUniform, 1);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    for (p = text; *p; p++) {
        int index = *p - 32;
        if (!textures[index]) {
            if (FT_Load_Char(face, *p, FT_LOAD_RENDER))
                continue;
            FT_GlyphSlot g = face->glyph;

            glGenTextures(1, &textures[index]);
            glBindTexture(GL_TEXTURE_2D, textures[index]);
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
                g->bitmap.buffer);
        } else {
            glBindTexture(GL_TEXTURE_2D, textures[index]);
        }
        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, text_model_matrix);

        text_model_matrix[12] += scale;
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    glBindVertexArray(0);
    glUniform1i(renderTextUniform, 0);
}

int init_text_renderer()
{
    if (FT_Init_FreeType(&ft)) {
		log_error("Could not init freetype library\n");
        return -1;
    }
    freetype_initialized = 1;
    if (FT_New_Face(ft, "./fonts/main.ttf", 0, &face)) {
		log_error("Could not open font\n");
        return -1;
    }

    FT_Set_Pixel_Sizes(face, 0, FONT_SIZE);
    GLuint program = renderer_get_main_program();
    glUniform1i(glGetUniformLocation(program, "tex"), 0);
    renderTextUniform = glGetUniformLocation(program, "renderText");
    modelMatrixUniform = glGetUniformLocation(program, "modelMatrix");
    // generate start screen text
    load_identity_matrix(text_model_matrix);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (char*)NULL + (sizeof(float) * 4));
    glEnableVertexAttribArray(3);
    glActiveTexture(GL_TEXTURE0);

    glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_STATIC_DRAW);
    glBindVertexArray(0);
    return 0;
}

void dispose_text_renderer()
{
    if (freetype_initialized) {
        FT_Done_FreeType(ft);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        for (int i = 0; i < NUM_LETTERS; i++) {
            if (textures[i])
                glDeleteTextures(1, &textures[i]);
        }
    }
}
