#ifndef _MESH_H_
#define _MESH_H_

#include <GL/glew.h>

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

extern PONG_ELEMENT player_stick;
extern PONG_ELEMENT opponent_stick;
extern PONG_ELEMENT ball;
extern PONG_ELEMENT stage;
extern PONG_ELEMENT ball_shadow;
extern PONG_ELEMENT stick_shadow;
extern PONG_ELEMENT ball_mark;
extern PONG_ELEMENT overlay;
extern PONG_ELEMENT startText;


void create_elements(float stage_width, float stage_height);
void dispose_elements();
void load_identity_matrix(float *out); 

#endif

