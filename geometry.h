#ifndef _MESH_H_
#define _MESH_H_

#include <GL/gl.h>

#define P_2PI 6.283185307f

// position*4 + color*4 + normal*4 + texture*2
#define VERTEX_SIZE 14


typedef struct {
  float* vertex;
  int vertex_count;
  GLuint vbo;
  GLuint texture;
  unsigned int* elements;
  int elements_count;
  GLuint mode;
  GLuint vertexType;
  float x;
  float y;
  float z;
  float xprev;
  float yprev;
  float zprev;
  GLuint vao;
  GLuint ebo;
  float width;
  float height;
  float large;
  float model_matrix[16];
}PONG_ELEMENT;

void create_geometry(float stage_width, float stage_height);

#endif
