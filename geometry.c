#include "geometry.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>


/**
  Game objects
 */

PONG_ELEMENT player_stick;
PONG_ELEMENT opponent_stick;
PONG_ELEMENT ball;
PONG_ELEMENT stage;
PONG_ELEMENT ball_shadow;
PONG_ELEMENT stick_shadow;
PONG_ELEMENT ball_mark;
PONG_ELEMENT overlay;
PONG_ELEMENT startText;


void normalize_vertex(float* mesh, int index, int count) {
	for (int i = 0; i < count; i++) {
		int offset = (i + index) * VERTEX_SIZE;
		float length = sqrt(mesh[offset] * mesh[offset] + mesh[offset + 1] * mesh[offset + 1] + mesh[offset + 2] * mesh[offset + 2]);
		mesh[offset] /= length;
		mesh[offset + 1] /= length;
		mesh[offset + 2] /= length;
	}
}


void create_projection_matrix(float fovy, float aspect_ratio, float near_plane, float far_plane, float* out) {
	const float
		y_scale = 1.0 / tan(M_PI / 180.0 * (fovy / 2.0)),
				x_scale = y_scale / aspect_ratio,
				frustum_length = far_plane - near_plane;

	out[1] = 0.0f;
	out[2] = 0.0f;
	out[3] = 0.0f;
	out[4] = 0.0f;
	out[6] = 0.0f;
	out[7] = 0.0f;
	out[8] = 0.0f;
	out[9] = 0.0f;
	out[12] = 0.0f;
	out[13] = 0.0f;
	out[15] = 0.0f;
	out[0] = x_scale;
	out[5] = y_scale;
	out[10] = -((far_plane + near_plane) / frustum_length);
	out[11] = -1;
	out[14] = -((2 * near_plane * far_plane) / frustum_length);
}

void load_identity_matrix(float *out) {
	out[0] = 1.0f;
	out[1] = 0.0f;
	out[2] = 0.0f;
	out[3] = 0.0f;
	out[4] = 0.0f;
	out[5] = 1.0f;
	out[6] = 0.0f;
	out[7] = 0.0f;
	out[8] = 0.0f;
	out[9] = 0.0f;
	out[10] = 1.0f;
	out[11] = 0.0f;
	out[12] = 0.0f;
	out[13] = 0.0f;
	out[14] = 0.0f;
	out[15] = 1.0f;
}

void assign_position_to_vertex(float* dest, int dest_index, float x, float y, float z) {
	int dest_offset = dest_index * VERTEX_SIZE;
	dest[dest_offset] = x;
	dest[dest_offset + 1] = y;
	dest[dest_offset + 2] = z;
	dest[dest_offset + 3] = 1.0f;
}

void cp_position_to_vertex(float* src, float* dest, int src_index, int dest_index) {
	int src_offset = src_index * 3;
	int dest_offset = dest_index * VERTEX_SIZE;
	dest[dest_offset] = src[src_offset];
	dest[dest_offset + 1] = src[src_offset + 1];
	dest[dest_offset + 2] = src[src_offset + 2];
	dest[dest_offset + 3] = 1.0f;
}

void assign_color_to_vertex(float* vertex_buffer, int index, float r, float g, float b, float a) {
	int offset = index * VERTEX_SIZE + 4;
	vertex_buffer[offset] = r;
	vertex_buffer[offset + 1] = g;
	vertex_buffer[offset + 2] = b;
	vertex_buffer[offset + 3] = a;
}
void assign_uv_to_vertex(float* vertex_buffer, int index, float u, float v) {
	int offset = index * VERTEX_SIZE + 12;
	vertex_buffer[offset] = u;
	vertex_buffer[offset + 1] = v;
}
/*
   Algorithm to emit triangles vertices for a common mesh
 */

void emit_mesh_triangle_pair(int index, int num_base_vertices, int* triangle1, int* triangle2) {
	int init_line = ((index + 1) % num_base_vertices) == 0;
	triangle1[0] = index;
	triangle1[1] = index + num_base_vertices;
	triangle1[2] = init_line ? index + 1 : index + (num_base_vertices + 1);

	triangle2[0] = index;
	triangle2[1] = init_line ? index + 1 : index + (num_base_vertices + 1);
	triangle2[2] = init_line ? index - (num_base_vertices - 1) : index + 1;
}

void setup_stick(PONG_ELEMENT* stick, float stick_width, float stick_height, const float* color) {

	stick->vertexType = GL_TRIANGLES;

	stick->width = stick_width;
	stick->height = stick_height;

	float width2 = stick->width / 2.0f;
	float height2 = stick->height / 2.0f;

	stick->vertex = (float*)calloc(4 * VERTEX_SIZE, sizeof(float));
	stick->vertex_count = 4;
	stick->elements = (unsigned int*)malloc(sizeof(unsigned int) * 6);
	stick->elements_count = 6;

	assign_position_to_vertex(stick->vertex, 0, -width2, -height2, stick->z);
	assign_color_to_vertex(stick->vertex, 0, color[0], color[1], color[2], color[3]);
	assign_uv_to_vertex(stick->vertex, 0, 0, 0);

	assign_position_to_vertex(stick->vertex, 1, -width2, height2, stick->z);
	assign_color_to_vertex(stick->vertex, 1, color[0], color[1], color[2], color[3]);
	assign_uv_to_vertex(stick->vertex, 1, 0, 1);

	assign_position_to_vertex(stick->vertex, 2, width2, height2, stick->z);
	assign_color_to_vertex(stick->vertex, 2, color[0], color[1], color[2], color[3]);
	assign_uv_to_vertex(stick->vertex, 2, 1, 1);

	assign_position_to_vertex(stick->vertex, 3, width2, -height2, stick->z);
	assign_color_to_vertex(stick->vertex, 3, color[0], color[1], color[2], color[3]);
	assign_uv_to_vertex(stick->vertex, 3, 1, 0);

	stick->elements[0] = 0;
	stick->elements[1] = 1;
	stick->elements[2] = 2;
	stick->elements[3] = 2;
	stick->elements[4] = 3;
	stick->elements[5] = 0;

	load_identity_matrix(stick->model_matrix);

}

void setup_overlay(PONG_ELEMENT* overlay, float alpha, float stage_width, float stage_height) {
	overlay->width = stage_width;
	overlay->height = stage_height;

	overlay->vertexType = GL_TRIANGLES;

	float width2 = overlay->width / 2.0f;
	float height2 = overlay->height / 2.0f;

	overlay->vertex = (float*)calloc(4 * VERTEX_SIZE, sizeof(float));
	overlay->vertex_count = 4;
	overlay->elements = (unsigned int*)malloc(sizeof(unsigned int) * 6);

	overlay->elements_count = 6;

	assign_position_to_vertex(overlay->vertex, 0, -width2, -height2, 0.0f);
	assign_color_to_vertex(overlay->vertex, 0, 0.0f, 0.0f, 0.0f, alpha);

	assign_position_to_vertex(overlay->vertex, 1, -width2, height2, 0.0f);
	assign_color_to_vertex(overlay->vertex, 1, 0.0f, 0.0f, 0.0f, alpha);

	assign_position_to_vertex(overlay->vertex, 2, width2, height2, 0.0f);
	assign_color_to_vertex(overlay->vertex, 2, 0.0f, 0.0f, 0.0f, alpha);

	assign_position_to_vertex(overlay->vertex, 3, width2, -height2, 0.0f);
	assign_color_to_vertex(overlay->vertex, 3, 0.0f, 0.0f, 0.0f, alpha);

	overlay->elements[0] = 0;
	overlay->elements[1] = 1;
	overlay->elements[2] = 2;
	overlay->elements[3] = 2;
	overlay->elements[4] = 3;
	overlay->elements[5] = 0;

	load_identity_matrix(overlay->model_matrix);
	overlay->model_matrix[14] = 0.0f;
}

void setup_stage(PONG_ELEMENT* stage,
		int window_width,
		int window_height,
		int blocks,
		float width,
		float large,
		const float* color) {

	int vertex = 0;
	float aspect = (float)window_width / window_height;
	float x_width, y_height;
	stage->width = width;
	stage->height = width / aspect;
	stage->large = large;

	stage->vertexType = GL_QUADS;

	x_width = stage->width / 2.0f;
	y_height = stage->height / 2.0f;

	stage->vertex_count =  blocks * 16;
	stage->elements_count = 0;

	stage->vertex = (float*)calloc( VERTEX_SIZE * stage->vertex_count, sizeof(float));

	// first create temporal vertices for 3d box (4 vetices up and 4 vertices bottom)
	int tmp_vertex_count = blocks * 8;
	float *tmp_vertex = (float*)malloc(sizeof(float) * 3 * tmp_vertex_count);

	for (int i = 0; i <= blocks; i++) {
		float z_depth  = (-stage->large * (float)i);

		tmp_vertex[vertex++] = -x_width;
		tmp_vertex[vertex++] = y_height;
		tmp_vertex[vertex++] = z_depth;

		tmp_vertex[vertex++] = x_width;
		tmp_vertex[vertex++] = y_height;
		tmp_vertex[vertex++] = z_depth;

		tmp_vertex[vertex++] = x_width;
		tmp_vertex[vertex++] = -y_height;
		tmp_vertex[vertex++] = z_depth;

		tmp_vertex[vertex++] = -x_width;
		tmp_vertex[vertex++] = -y_height;
		tmp_vertex[vertex++] =  z_depth;
	}

	// now join vertices for build quads using intermediate triangles
	int triangle1[3];
	int triangle2[3];
	vertex = 0;
	float alpha = color[3];

	for (int i = 0, j = 0; i < blocks * 4; i++, j += 4) {
		emit_mesh_triangle_pair(i, 4, triangle1, triangle2);
		// each block has got a darker color
		if (i > 0 && i % 4 == 0) {
			alpha /= 2.0f;
		}
		cp_position_to_vertex(tmp_vertex, stage->vertex, triangle1[0], j);
		assign_color_to_vertex(stage->vertex, j, color[0], color[1], color[2], color[3]);

		cp_position_to_vertex(tmp_vertex, stage->vertex, triangle1[1], j + 1);
		assign_color_to_vertex(stage->vertex, j + 1, color[0], color[1], color[2], color[3]);

		cp_position_to_vertex(tmp_vertex, stage->vertex, triangle1[2], j + 2);
		assign_color_to_vertex(stage->vertex, j + 2, color[0], color[1], color[2], color[3]);

		cp_position_to_vertex(tmp_vertex, stage->vertex, triangle2[2], j + 3);
		assign_color_to_vertex(stage->vertex, j + 3, color[0], color[1], color[2], color[3]);
	}
	free(tmp_vertex);
	load_identity_matrix(stage->model_matrix);
}

void setup_ball(PONG_ELEMENT* ball, int segments, float radius, const float* color) {
	float theta;
	float phi;
	int vertex = 0;
	int m, p;
	int triangle1[3];
	int triangle2[3];
	float unit_angle = P_2PI / segments;

	ball->vertexType = GL_TRIANGLES;
	ball->width = segments;

	ball->vertex_count = segments * segments;
	ball->elements_count = (ball->vertex_count * 6 + 6);

	ball->vertex = (float*)calloc(VERTEX_SIZE * ball->vertex_count, sizeof(float));
	ball->elements = (unsigned int*)malloc(sizeof(unsigned int) * ball->elements_count);


	for (p = 0, theta = -M_PI_2; p < segments; p++, theta += unit_angle)
	{
		for (m = 0, phi = 0.0f; m < segments; m++, phi += unit_angle)
		{
			assign_position_to_vertex(
					ball->vertex, vertex,
					cos(theta) * sin(phi) * ball->width,
					sin(theta) * ball->width,
					cos(theta) * cos(phi) * ball->width
					);
			assign_color_to_vertex(ball->vertex, vertex, color[0], color[1], color[2], color[3]);
			vertex++;
		}
	}
	for (int i = 0, j = 0; i < (ball->vertex_count >> 1); i++, j += 6) {
		emit_mesh_triangle_pair(i, segments, triangle1, triangle2);
		ball->elements[j] = triangle1[0];
		ball->elements[j + 1] = triangle1[1];
		ball->elements[j + 2] = triangle1[2];
		ball->elements[j + 3] = triangle2[0];
		ball->elements[j + 4] = triangle2[1];
		ball->elements[j + 5] = triangle2[2];
	}

	load_identity_matrix(ball->model_matrix);
}

void build_circle(PONG_ELEMENT* element, float radius, int segments, const float* color) {
	element->vertex = (float*)calloc((segments + 2) * VERTEX_SIZE, sizeof(float));
	element->vertex_count = (segments + 2);
	element->elements_count = 0;
	element->vertexType = GL_TRIANGLE_FAN;
	int vertex = 1;
	int p;
	float unit_angle = P_2PI / (float)segments;
	float theta;
	element->width = radius;
	assign_position_to_vertex(element->vertex, 0, 0.0f, 0.0f, 0.0f);
	assign_color_to_vertex(element->vertex, 0, color[0], color[1], color[2], color[3]);
	for (p = 0, theta = 0; p <= segments; p++, theta -= unit_angle) {
		assign_position_to_vertex(element->vertex, vertex, 0.0f, sin(theta) * element->width, -cos(theta) * element->width);
		assign_color_to_vertex(element->vertex, vertex, color[0], color[1], color[2], color[3]);
		vertex++;
	}
	load_identity_matrix(element->model_matrix);
}
void setup_ball_shadow(PONG_ELEMENT* element, float radius, int segments, const float* color) {
	load_identity_matrix(element->model_matrix);
	build_circle(element, radius, segments, color);
}

void setup_ball_marks(PONG_ELEMENT* element, float radius, int segments, const float* color) {
	build_circle(element, radius / 2.0f, segments, color);
}
void setup_stick_shadows(PONG_ELEMENT* element, float width, float height, const float* color) {

	int elements[] = {0, 1, 2, 0, 2, 3};

	element->vertex = (float*)calloc(4 * VERTEX_SIZE, sizeof(float));
	element->elements = (unsigned int*)malloc(sizeof(unsigned int) * 6);
	element->vertex_count = 4;
	element->elements_count = 6;
	element->width = width;
	element->height = height;
	element->z = 0.0f;
	element->vertexType = GL_TRIANGLES;
	float width2 = element->width / 2.0f;
	float height2 = element->height / 2.0f;

	assign_position_to_vertex(element->vertex, 0, -width2, -height2, element->z);
	assign_color_to_vertex(element->vertex, 0, color[0], color[1], color[2], color[3]);

	assign_position_to_vertex(element->vertex, 1, -width2, height2, element->z);
	assign_color_to_vertex(element->vertex, 1, color[0], color[1], color[2], color[3]);

	assign_position_to_vertex(element->vertex, 2, width2, height2, element->z);
	assign_color_to_vertex(element->vertex, 2, color[0], color[1], color[2], color[3]);

	assign_position_to_vertex(element->vertex, 3, width2, -height2, element->z);
	assign_color_to_vertex(element->vertex, 3, color[0], color[1], color[2], color[3]);

	memcpy(element->elements, elements, sizeof(elements));
	load_identity_matrix(element->model_matrix);
}


void create_elements(const float window_width, const float window_height)  {
	/**
	  Config for geometry of all objects in game
	 */
	float stage_width = 1.0f;
	float stage_blocks = 7;
	float blocks_large = stage_width / 5.0f;
	float stage_color[] = { 0.0, 1.0, 0.0, 0.2 };
	float overlay_alpha = 0.8f;
	float aspect = (float)window_width / window_height;
	float stick_width = stage_width / 6.0f;
	float stick_color[] = { 0.5, 0.5, 0.5, 0.5 }; 
	float ball_segments = 20;
	float ball_radius = stage_width / 50.0f;
	float ball_color[4] = { 1.0 };
	float shadows_color[] = { 1.0, 1.0, 1.0, 0.2f };

	// default geometry values

	setup_stage(&stage, window_width, window_height, stage_blocks, stage_width, blocks_large, stage_color);

	// setup_stage sets height of stage in relation to window aspect.
	// height is read from stage object in last parameter.
	setup_overlay(&overlay, overlay_alpha, stage_width, stage.height);

	setup_stick(&player_stick, stick_width, stick_width * aspect, stick_color);
	setup_stick(&opponent_stick, stick_width, stick_width * aspect, stick_color);
	setup_ball(&ball, ball_segments, ball_radius, ball_color);
	setup_ball_shadow(&ball_shadow, ball_segments, ball_radius, shadows_color);
	setup_ball_marks(&ball_mark, ball_segments, ball_radius / 2.0f, ball_color);
	setup_stick_shadows(&stick_shadow, stick_width, stick_width / 50.0f, shadows_color);
}

void free_pong_element(PONG_ELEMENT* element) {
  free(element->vertex);
  if (element->elements_count > 0) {
    free(element->elements);
  }
}

void dispose_elements() {
  free_pong_element(&player_stick);
  free_pong_element(&opponent_stick);
  free_pong_element(&ball);
  free_pong_element(&stage);
  free_pong_element(&ball_shadow);
  free_pong_element(&stick_shadow);
  free_pong_element(&ball_mark);
}
