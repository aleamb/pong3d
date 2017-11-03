/**
	@file geometry.c
	@author Alejandro Ambroa
	@date 1 Oct 2017
	@brief Game objects for pong3d and utils functions for geometry tranformations.
*/
#include "geometry.h"
#include "renderer.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OVERLAY_ALPHA 0.8f

PONG_ELEMENT player_stick;
PONG_ELEMENT opponent_stick;
PONG_ELEMENT ball;
PONG_ELEMENT stage;
PONG_ELEMENT ball_shadow;
PONG_ELEMENT stick_shadow;
PONG_ELEMENT ball_mark;
PONG_ELEMENT overlay;
PONG_ELEMENT startText;

float opponent_z_coord;

void create_projection_matrix(float fovy, float aspect_ratio, float near_plane, float far_plane, float* out)
{
    const float
        y_scale
        = 1.0f / (float)tan(M_PI / 180.0f * (fovy / 2.0f)),
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

void load_identity_matrix(float* out)
{
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

void assign_position_to_vertex(float* dest, int dest_index, float x, float y, float z)
{
    int dest_offset = dest_index * VERTEX_SIZE;
    dest[dest_offset] = x;
    dest[dest_offset + 1] = y;
    dest[dest_offset + 2] = z;
    dest[dest_offset + 3] = 1.0f;
}

void cp_position_to_vertex(float* src, float* dest, int src_index, int dest_index)
{
    int src_offset = src_index * 3;
    int dest_offset = dest_index * VERTEX_SIZE;
    dest[dest_offset] = src[src_offset];
    dest[dest_offset + 1] = src[src_offset + 1];
    dest[dest_offset + 2] = src[src_offset + 2];
    dest[dest_offset + 3] = 1.0f;
}

void assign_color_to_vertex(float* vertex_buffer, int index, float r, float g, float b, float a)
{
    int offset = index * VERTEX_SIZE + 4;
    vertex_buffer[offset] = r;
    vertex_buffer[offset + 1] = g;
    vertex_buffer[offset + 2] = b;
    vertex_buffer[offset + 3] = a;
}
void assign_uv_to_vertex(float* vertex_buffer, int index, float u, float v)
{
    int offset = index * VERTEX_SIZE + 12;
    vertex_buffer[offset] = u;
    vertex_buffer[offset + 1] = v;
}

/*
   Algorithm to build vertex indices of common mesh.
*/

void emit_mesh_triangle_pair(int index, int num_base_vertices, int* triangle1, int* triangle2)
{
    int init_line = ((index + 1) % num_base_vertices) == 0;
    triangle1[0] = index;
    triangle1[1] = index + num_base_vertices;
    triangle1[2] = init_line ? index + 1 : index + (num_base_vertices + 1);

    triangle2[0] = index;
    triangle2[1] = init_line ? index + 1 : index + (num_base_vertices + 1);
    triangle2[2] = init_line ? index - (num_base_vertices - 1) : index + 1;
}

void setup_stick(PONG_ELEMENT* stick, float stick_width, float stick_height, const float* color)
{

    stick->vertexType = GL_TRIANGLES;

    stick->width = stick_width;
    stick->height = stick_height;

    stick->width2 = stick->width / 2.0f;
    stick->height2 = stick->height / 2.0f;

    stick->vertex = (float*)calloc(4 * VERTEX_SIZE, sizeof(float));
    stick->vertex_count = 4;
    stick->elements = (unsigned int*)malloc(sizeof(unsigned int) * 6);
    stick->elements_count = 6;

    assign_position_to_vertex(stick->vertex, 0, -stick->width2, -stick->height2, 0.0f);
    assign_color_to_vertex(stick->vertex, 0, color[0], color[1], color[2], color[3]);
    assign_uv_to_vertex(stick->vertex, 0, 0, 0);

    assign_position_to_vertex(stick->vertex, 1, -stick->width2, stick->height2, 0.0f);
    assign_color_to_vertex(stick->vertex, 1, color[0], color[1], color[2], color[3]);
    assign_uv_to_vertex(stick->vertex, 1, 0, 1);

    assign_position_to_vertex(stick->vertex, 2, stick->width2, stick->height2, 0.0f);
    assign_color_to_vertex(stick->vertex, 2, color[0], color[1], color[2], color[3]);
    assign_uv_to_vertex(stick->vertex, 2, 1, 1);

    assign_position_to_vertex(stick->vertex, 3, stick->width2, -stick->height2, 0.0f);
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

void setup_overlay(PONG_ELEMENT* overlay, float alpha, float stage_width, float stage_height)
{
    overlay->width = stage_width;
    overlay->height = stage_height;

    overlay->vertexType = GL_TRIANGLES;

    overlay->width2 = overlay->width / 2.0f;
    overlay->height2 = overlay->height / 2.0f;

    overlay->vertex = (float*)calloc(4 * VERTEX_SIZE, sizeof(float));
    overlay->vertex_count = 4;
    overlay->elements = (unsigned int*)malloc(sizeof(unsigned int) * 6);

    overlay->elements_count = 6;

    assign_position_to_vertex(overlay->vertex, 0, -overlay->width2, -overlay->height2, 0.0f);
    assign_color_to_vertex(overlay->vertex, 0, 0.0f, 0.0f, 0.0f, alpha);

    assign_position_to_vertex(overlay->vertex, 1, -overlay->width2, overlay->height2, 0.0f);
    assign_color_to_vertex(overlay->vertex, 1, 0.0f, 0.0f, 0.0f, alpha);

    assign_position_to_vertex(overlay->vertex, 2, overlay->width2, overlay->height2, 0.0f);
    assign_color_to_vertex(overlay->vertex, 2, 0.0f, 0.0f, 0.0f, alpha);

    assign_position_to_vertex(overlay->vertex, 3, overlay->width2, -overlay->height2, 0.0f);
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
    const float* color)
{

    int vertex = 0;
    float aspect = (float)window_width / window_height;
    float x_width, y_height;
    stage->width = width;
    stage->height = width / aspect;
    stage->large = large * blocks;
    stage->width2 = stage->width / 2.0f;
    stage->height2 = stage->height / 2.0f;

    stage->vertexType = GL_QUADS;

    x_width = stage->width2;
    y_height = stage->height2;

    stage->vertex_count = blocks * 16;
    stage->elements_count = 0;

    stage->vertex = (float*)calloc(VERTEX_SIZE * stage->vertex_count, sizeof(float));

    // first create temporal vertices for 3d box (4 vetices up and 4 vertices bottom)
    int tmp_vertex_count = blocks * 8;
    float* tmp_vertex = (float*)malloc(sizeof(float) * 3 * tmp_vertex_count);

    for (int i = 0; i <= blocks; i++) {
        float z_depth = (-large * (float)i);

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
        tmp_vertex[vertex++] = z_depth;
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
        assign_color_to_vertex(stage->vertex, j, color[0], color[1], color[2], alpha);

        cp_position_to_vertex(tmp_vertex, stage->vertex, triangle1[1], j + 1);
        assign_color_to_vertex(stage->vertex, j + 1, color[0], color[1], color[2], alpha);

        cp_position_to_vertex(tmp_vertex, stage->vertex, triangle1[2], j + 2);
        assign_color_to_vertex(stage->vertex, j + 2, color[0], color[1], color[2], alpha);

        cp_position_to_vertex(tmp_vertex, stage->vertex, triangle2[2], j + 3);
        assign_color_to_vertex(stage->vertex, j + 3, color[0], color[1], color[2], alpha);
    }
    free(tmp_vertex);
    load_identity_matrix(stage->model_matrix);
}

void setup_ball(PONG_ELEMENT* ball, int segments, float radius, const float* color)
{
    float theta;
    float phi;
    int vertex = 0;
    int m, p;
    int triangle1[3];
    int triangle2[3];
    float unit_angle = P_2PI / segments;

    ball->vertexType = GL_TRIANGLES;
    ball->width = radius;

    ball->vertex_count = segments * segments;
    ball->elements_count = (ball->vertex_count * 6);

    ball->vertex = (float*)calloc(VERTEX_SIZE * ball->vertex_count, sizeof(float));
    ball->elements = (unsigned int*)calloc(ball->elements_count, sizeof(unsigned int));

    for (p = 0, theta = (float)-M_PI_2; p < segments; p++, theta += unit_angle) {
        for (m = 0, phi = 0.0f; m < segments; m++, phi += unit_angle) {
            assign_position_to_vertex(
                ball->vertex, vertex,
                cos(theta) * sin(phi) * ball->width,
                sin(theta) * ball->width,
                cos(theta) * cos(phi) * ball->width);
            assign_color_to_vertex(ball->vertex, vertex, color[0], color[1], color[2], color[3]);
            vertex++;
        }
    }
    for (int i = 0, j = 0; i < (ball->vertex_count - segments); i++, j += 6) {
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

void build_circle(PONG_ELEMENT* element, float radius, int segments, const float* color)
{
    element->vertex_count = (segments + 2);
    element->vertex = (float*)calloc(element->vertex_count * VERTEX_SIZE, sizeof(float));
    element->elements_count = 0;
    element->vertexType = GL_TRIANGLE_FAN;
    int vertex = 1;
    int p;
    float unit_angle = P_2PI / (float)segments;
    float theta;
    element->width = radius;

    assign_position_to_vertex(element->vertex, 0, 0.0f, 0.0f, 0.0f);
    assign_color_to_vertex(element->vertex, 0, color[0], color[1], color[2], color[3]);

    for (p = 0, theta = 0; p < element->vertex_count - 1; p++, theta -= unit_angle) {
        assign_position_to_vertex(element->vertex, vertex, cos(theta) * element->width, sin(theta) * element->width, 0);
        assign_color_to_vertex(element->vertex, vertex, color[0], color[1], color[2], color[3]);
        vertex++;
    }

    load_identity_matrix(element->model_matrix);
    element->model_matrix[12] = -0.2f;
}
void setup_ball_shadow(PONG_ELEMENT* element, int segments, float radius, const float* color)
{
    load_identity_matrix(element->model_matrix);
    build_circle(element, radius, segments, color);
}

void setup_ball_marks(PONG_ELEMENT* element, int segments, float radius, const float* color)
{
    load_identity_matrix(element->model_matrix);
    build_circle(element, radius, segments, color);
    element->model_matrix[14] = 0.02f;
}
void setup_stick_shadows(PONG_ELEMENT* element, float width, float height, const float* color)
{

    int elements[] = { 0, 1, 2, 2, 3, 0 };

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
    element->width2 = width2;
    element->height2 = height2;

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

void create_elements(int window_width, int window_height, int stage_blocks)
{
    /**
	  Config for geometry of all objects in game
	  : */
    float stage_width = 1.0f;
    float stage_large = 1.5f;
    float blocks_large = stage_large / stage_blocks;

    float stage_color[] = { 0.0f, 1.0f, 0.0f, 0.2f };
    float overlay_alpha = OVERLAY_ALPHA;
    float aspect = (float)window_width / window_height;
    float stick_width = stage_width / 6.0f;
    float stick_color[] = { 0.5f, 0.5f, 0.5f, 0.5f };
    int ball_segments = 20;
    float ball_radius = stage_large / 80.0f;
    float ball_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float shadows_color[] = { 1.0f, 1.0f, 1.0f, 0.2f };
    opponent_z_coord = -stage_blocks * blocks_large;

    setup_stage(&stage, window_width, window_height, stage_blocks, stage_width, blocks_large, stage_color);

    // setup_stage sets height of stage in relation to window aspect.
    // height is read from stage object in last parameter.
    setup_overlay(&overlay, overlay_alpha, stage_width, stage.height);

    setup_stick(&opponent_stick, stick_width, stick_width / aspect, stick_color);

    setup_ball(&ball, ball_segments, ball_radius, ball_color);

    setup_ball_shadow(&ball_shadow, ball_segments, ball_radius, shadows_color);

    setup_ball_marks(&ball_mark, ball_segments, ball_radius / 2.0f, ball_color);

    setup_stick(&player_stick, stick_width, stick_width / aspect, stick_color);

    setup_stick_shadows(&stick_shadow, stick_width, ball_radius, shadows_color);

    reset_player_stick_position();
    reset_opponent_stick_position();
    upload_to_renderer(&stage);
    upload_to_renderer(&overlay);
    upload_to_renderer(&player_stick);
    upload_to_renderer(&opponent_stick);
    upload_to_renderer(&ball);
    upload_to_renderer(&ball_shadow);
    upload_to_renderer(&ball_mark);
    upload_to_renderer(&stick_shadow);
}

void free_pong_element(PONG_ELEMENT* element)
{
    if (element->vertex) {
        free(element->vertex);
    }
    if (element->elements_count > 0 && element->elements) {
        free(element->elements);
    }
    remove_to_renderer(element);
}

void dispose_elements()
{
    free_pong_element(&player_stick);
    free_pong_element(&opponent_stick);
    free_pong_element(&ball);
    free_pong_element(&stage);
    free_pong_element(&ball_shadow);
    free_pong_element(&stick_shadow);
    free_pong_element(&ball_mark);
}

void reset_player_stick_position()
{
    player_stick.x = 0.0f;
    player_stick.y = 0.0f;
    player_stick.xprev = 0.0f;
    player_stick.yprev = 0.0f;
    player_stick.z = 0.0f;
}
void reset_opponent_stick_position()
{
    opponent_stick.x = 0.0f;
    opponent_stick.y = 0.0f;
    opponent_stick.xprev = 0.0f;
    opponent_stick.yprev = 0.0f;
    opponent_stick.z = opponent_z_coord;
    opponent_stick.model_matrix[12] = 0.0;
    opponent_stick.model_matrix[13] = 0.0;
    opponent_stick.model_matrix[14] = opponent_stick.z;
}
void move_player_stick(float x, float y)
{
    player_stick.x = x;
    player_stick.y = y;
    player_stick.model_matrix[12] = x;
    player_stick.model_matrix[13] = y;
}
void move_opponent_stick(float x, float y)
{
    opponent_stick.x = x;
    opponent_stick.y = y;
    opponent_stick.model_matrix[12] = x;
    opponent_stick.model_matrix[13] = y;
}
void reset_ball_position()
{
    ball.x = 0.0f;
    ball.y = 0.0f;
    ball.z = player_stick.z - ball.width;
    ball.model_matrix[12] = 0.0;
    ball.model_matrix[13] = 0.0;
    ball.model_matrix[14] = ball.z;
}
void move_ball(float x, float y, float z)
{
    ball.x = x;
    ball.y = y;
    ball.z = z;
    ball.model_matrix[12] = x;
    ball.model_matrix[13] = y;
    ball.model_matrix[14] = z;
}
