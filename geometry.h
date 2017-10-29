/**
  @file geometry.h
  @author Alejandro Ambroa
  @date 1 Oct 2017
  @brief Game objects for pong3d and utils functions for geometry tranformations. 
 */

#ifndef _MESH_H_
#define _MESH_H_

#include <GL/glew.h>

#define P_2PI 6.283185307f

/**
  @brief Vertex structure is position * 4 + color * 4 + normal * 4 + texture * 2. Float types.
 */
#define VERTEX_SIZE 14

/**
  @brief Game object structure.
 */
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
	/** is useful to keep half of sizes for avoid divisions in rendering code. */
	float width2;
	float height2;
	float large2;
	/** model matrix to apply tranformations */
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


void create_elements(float stage_width, float stage_height, int num_blocks);
void dispose_elements();
void load_identity_matrix(float *out);
void create_projection_matrix(float fovy, float aspect_ratio, float near_plane, float far_plane, float* out); 
void reset_player_stick_position();
void reset_opponent_stick_position();
void move_player_stick(float x, float y);
void move_opponent_stick(float x, float y);
void reset_ball_position();
void move_ball(float x, float y, float z);

#endif

