#include <SDL2/SDL.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <ft2build.h>
#include FT_FREETYPE_H


#define ERRORMSG_MAX_LENGTH 256

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 378

#define STAGE_BLOCKS 7
#define BALL_SEGMENTS 20

#define STAGE_BLOCK_WIDTH 1.0f
#define STAGE_BLOCK_LARGE (STAGE_BLOCK_WIDTH / 5.0f)
#define STICK_WIDTH (STAGE_BLOCK_WIDTH / 6.0f)
#define BALL_RADIUS (STAGE_BLOCK_WIDTH / 50.0f)
#define VELOCITY_SAMPLE_F 550
#define OPPONENT_SAMPLE_ADJUST 80
#define SAMPLE_FREQ 44100

#define STAGE_VERTICES_COUNT (4 * STAGE_BLOCKS + 4)
#define STAGE_INDICES_COUNT (STAGE_BLOCKS * 24)
#define BALL_VERTICES_COUNT (BALL_SEGMENTS * BALL_SEGMENTS)
#define BALL_INDICES_COUNT (BALL_SEGMENTS * BALL_SEGMENTS * 6 + 6)
#define P_2PI 6.283185307f
#define UNIT_ANGLE (P_2PI / BALL_SEGMENTS)

const float INITIAL_BALL_SPEED_VECTOR[] = { -0.1f,  0.05f, -1.3f };
const float OPP_STICK_RETURN_SPEED_VECTOR[] = { -0.1f,  0.1f, 0.0f };
const float OPP_STICK_SPEED_VECTOR[] = { -0.15f,  0.15f, 0.0f };


SDL_Window* window;
SDL_GLContext mainContext;

typedef struct {
  float* vertex;
  unsigned int* elements;
  int vertex_count;
  int elements_count;
  float x;
  float y;
  float z;
  float xprev;
  float yprev;
  float zprev;
  float wx;
  float wy;
  GLuint vao;
  GLuint vbo;
  GLuint ebo;
  float width;
  float height;
  float large;
  float model_matrix[16];
}PONG_ELEMENT;

PONG_ELEMENT player_stick;
PONG_ELEMENT enemy_stick;
PONG_ELEMENT ball;
PONG_ELEMENT stage;
PONG_ELEMENT ball_shadow;
PONG_ELEMENT stick_shadow;
PONG_ELEMENT ball_mark;

GLuint vertex_shader;
GLuint fragment_shader;
GLuint program;

GLuint projectionMatrixId;
GLuint viewMatrixId;
GLuint modelMatrixId;
GLuint projectionFlagId;
GLchar errormsg[ERRORMSG_MAX_LENGTH];

float ball_speed_vector[3];

const GLchar* vertex_shader_source = "#version 430 core\n \
in vec3 in_Position;\n \
in vec4 ex_Color;\n \
in vec2 uv;\n \
uniform mat4 projectionMatrix;\n \
uniform mat4 viewMatrix;\n \
uniform mat4 modelMatrix;\n \
void main(void) {\n \
  gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Position, 1.0);\n \
}";

const GLchar* fragment_shader_source = "#version 430 core\n \
precision highp float;\n \
void main(void) {\n \
  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n \
}";

float projection_matrix[16];
float view_matrix[16];
float scale_matrix[16];

SDL_AudioSpec want, have;
SDL_AudioDeviceID dev;

float* player_pong_sound;
int player_pong_sound_samples;

float* opponent_pong_sound;
int opponent_pong_sound_samples;

float* player_score_sound;
int player_score_sound_samples;

float* opp_score_sound;
int opp_score_sound_samples;

float* start_sound;
int start_sound_samples;

float* wall_hit_sound;
int wall_hit_sound_samples;

typedef enum {
    NONE,
    SIN,
    SAW,
    TRIANGLE,
    COS
}OSCILLATOR_TYPE;

typedef struct {
  float totalTime;
  float volume;
  float attackTime;
  float decayTime;
  float releaseTime;
  float decayValue;
  float filterBeta1;
  float filterBeta2;
  OSCILLATOR_TYPE oscillator1_type;
  OSCILLATOR_TYPE oscillator2_type;
  float oscillator1_freq;
  float oscillator2_freq;
  float delayTime;
  float reverbSize;
} SYNTH;

FT_Library ft;
FT_Face face;

int player_score;
int enemy_score;
int balls = 9;

int setup_screen(int width, int height) {

  if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
    fprintf( stderr, "Video initialization failed: %s\n", SDL_GetError( ) );
    return -1;
  }
	window = SDL_CreateWindow("Pong 3D",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		width, height, SDL_WINDOW_OPENGL);

  if (!window) {
    fprintf( stderr, "Window set failed: %s\n", SDL_GetError( ) );
    return -1;
  }

  mainContext = SDL_GL_CreateContext(window);

  if (!mainContext) {
    fprintf( stderr, "GL context creation failed: %s\n", SDL_GetError( ) );
    return -1;
  }

  SDL_ShowCursor(SDL_DISABLE);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  return 0;
}

void normalize(float* mesh, int size) {
  for (int i = 0; i < size; i += 3) {
    float length = sqrt(mesh[i] * mesh[i] + mesh[i + 1] * mesh[i + 1] + mesh[i + 2] * mesh[i + 2]);
    mesh[i] /= length;
    mesh[i + 1] /= length;
    mesh[i + 2] /= length;
  }
}
void create_vao(PONG_ELEMENT* element) {
  glGenVertexArrays(1, &element->vao);
  glBindVertexArray(element->vao);

  glGenBuffers(1, &element->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, element->vbo);
  glBufferData(GL_ARRAY_BUFFER, element->vertex_count * 3 * sizeof(float), element->vertex, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  if (element->elements_count > 0) {
    glGenBuffers(1, &element->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, element->elements_count * sizeof(unsigned int), element->elements, GL_STATIC_DRAW);
  }
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

GLuint build_shader(GLenum type, const GLchar* source, GLint* result, GLchar *errormsg) {
  GLuint id = glCreateShader(type);
  glShaderSource(id, 1, &source, NULL);
  glCompileShader(id);
  int params;
  glGetShaderiv(id, GL_COMPILE_STATUS, &params);
  if (params == GL_FALSE) {
    *result = -1;
    if (errormsg != NULL) {
        int maxLength;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);
        glGetShaderInfoLog(id,
          (maxLength > ERRORMSG_MAX_LENGTH ? ERRORMSG_MAX_LENGTH : maxLength), &maxLength, errormsg);
    }
    return -1;
  } else {
    *result = 0;
  }
  return id;
}

GLuint build_shaders_program(int count, int* result, GLchar* errormsg, ...) {
    va_list ap;
    GLuint program_id = glCreateProgram();
    va_start(ap, errormsg);
    for(int j = 0; j < count; j++) {
      GLuint shader = va_arg(ap, GLuint);
      glAttachShader(program_id, shader);
    }
    va_end(ap);
    glLinkProgram(program_id);
    GLint params;
    glGetProgramiv(program_id, GL_LINK_STATUS, &params);
    if (params == GL_FALSE) {
      *result = -1;
      if (errormsg != NULL) {
        int maxLength;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &maxLength);
        glGetProgramInfoLog(program_id,
          (maxLength > ERRORMSG_MAX_LENGTH ? ERRORMSG_MAX_LENGTH : maxLength), &maxLength, errormsg);
      }
    } else {
      *result = 0;
    }
    return program_id;
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


int setup_renderer(int width, int height) {

  GLenum err = glewInit();
  if (err != GLEW_OK) {
    fprintf(stderr, "OpenGL error: %s\n", glewGetErrorString(err));
    return -1;
  }
  glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
  glDisable(GL_CULL_FACE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  int result;
  vertex_shader = build_shader(GL_VERTEX_SHADER, vertex_shader_source, &result, errormsg);
  if (result != 0) {
      fprintf( stderr, "Compile vertex shader failed: %s\n", errormsg);
      return -1;
  }
  fragment_shader = build_shader(GL_FRAGMENT_SHADER, fragment_shader_source, &result, errormsg);
  if (result != 0) {
      fprintf( stderr, "Compile fragment shader failed: %s\n", errormsg);
      return -1;
  }
  program = build_shaders_program(2, &result, errormsg, vertex_shader, fragment_shader);
  if (result != 0) {
      fprintf( stderr, "Links shaders program failed: %s\n", errormsg);
      return -1;
  }

  glBindAttribLocation(program, 0, "in_Position");
  glBindAttribLocation(program, 1, "ex_Color");
  glBindAttribLocation(program, 2, "uv");

  glUseProgram(program);

  create_vao(&player_stick);
  create_vao(&enemy_stick);
  create_vao(&ball);
  create_vao(&stage);
  create_vao(&ball_shadow);
  create_vao(&ball_mark);
  create_vao(&stick_shadow);

  load_identity_matrix(view_matrix);

  // perspective Z compensation: 0.5 / tan(fov / 2) / aspect
  view_matrix[14] = -0.866f / ((float)WINDOW_WIDTH / WINDOW_HEIGHT);
  create_projection_matrix(60.0f, (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.001f, 10.0f, projection_matrix);
  projectionMatrixId = glGetUniformLocation(program, "projectionMatrix");
  viewMatrixId = glGetUniformLocation(program, "viewMatrix");
  modelMatrixId = glGetUniformLocation(program, "modelMatrix");
  projectionFlagId = glGetUniformLocation(program, "project");
  glUniformMatrix4fv(projectionMatrixId, 1, GL_FALSE, projection_matrix);
  glUniformMatrix4fv(viewMatrixId, 1, GL_FALSE, view_matrix);

  return 0;
}


void setup_stick(PONG_ELEMENT* stick) {

  float aspect = (float)WINDOW_WIDTH / WINDOW_HEIGHT;

  stick->width = STICK_WIDTH;
  stick->height = STICK_WIDTH / aspect;

  float width2 = stick->width / 2.0f;
  float height2 = stick->height / 2.0f;

  stick->vertex = (float*)malloc(sizeof(float) * 12);
  stick->vertex_count = 4;
  stick->elements = (unsigned int*)malloc(sizeof(unsigned int) * 6);

  stick->elements_count = 6;
  stick->vertex[0] = -width2;
  stick->vertex[1] = -height2;
  stick->vertex[2] = stick->z;

  stick->vertex[3] = -width2;
  stick->vertex[4] = height2;
  stick->vertex[5] = stick->z;

  stick->vertex[6] = width2;
  stick->vertex[7] = height2;
  stick->vertex[8] = stick->z;

  stick->vertex[9] = width2;
  stick->vertex[10] = -height2;
  stick->vertex[11] = stick->z;

  stick->elements[0] = 0;
  stick->elements[1] = 1;
  stick->elements[2] = 2;
  stick->elements[3] = 2;
  stick->elements[4] = 3;
  stick->elements[5] = 0;

  load_identity_matrix(stick->model_matrix);

}

void setup_player_stick() {
  player_stick.z = -STAGE_BLOCK_WIDTH / 50.0f;
  setup_stick(&player_stick);
}

void setup_enemy_stick() {
  enemy_stick.z = -(STAGE_BLOCKS * STAGE_BLOCK_LARGE);
  setup_stick(&enemy_stick);
}

void setup_stage() {

  int vertex = 0;
  float aspect = (float)WINDOW_WIDTH / WINDOW_HEIGHT;
  float x_width, y_height;
  stage.width = STAGE_BLOCK_WIDTH;
  stage.height = STAGE_BLOCK_WIDTH / aspect;
  stage.large = STAGE_BLOCK_LARGE;

  x_width = stage.width / 2.0f;
  y_height = stage.height / 2.0f;
  stage.vertex = (float*)malloc(sizeof(float) * 3 * STAGE_VERTICES_COUNT);
  stage.elements = (unsigned int*)malloc(sizeof(unsigned int) * STAGE_INDICES_COUNT);

  stage.vertex_count = STAGE_VERTICES_COUNT;
  stage.elements_count = STAGE_INDICES_COUNT;

  for (int i = 0; i <= STAGE_BLOCKS; i++) {
    float z_depth  = (-stage.large * (float)i);
    stage.vertex[vertex++] = -x_width;
    stage.vertex[vertex++] = y_height;
    stage.vertex[vertex++] = z_depth;

    stage.vertex[vertex++] = x_width;
    stage.vertex[vertex++] = y_height;
    stage.vertex[vertex++] = z_depth;

    stage.vertex[vertex++] = x_width;
    stage.vertex[vertex++] = -y_height;
    stage.vertex[vertex++] = z_depth;

    stage.vertex[vertex++] = -x_width;
    stage.vertex[vertex++] = -y_height;
    stage.vertex[vertex++] =  z_depth;
  }

  for (int i = 0, j = 0; i < (STAGE_VERTICES_COUNT - 4); i++, j += 6) {
      stage.elements[j] = i;
      stage.elements[j + 1] = i + 4;
      stage.elements[j + 2] = ((i + 1) % 4) == 0 ? i + 1 : i + 5;

      stage.elements[j + 3] = i;
      stage.elements[j + 4] = ((i + 1) % 4) == 0 ? i + 1 : i + 5;
      stage.elements[j + 5] = ((i + 1) % 4) == 0 ? i - 3 : i + 1;
  }
  load_identity_matrix(stage.model_matrix);
}

void setup_ball() {

  int initial_index[] = {0, 5, 6, 6, 1, 0};
  float theta;
  float phi;
  int vertex = 0;
  int m, p;
  ball.width = BALL_RADIUS;

  ball.vertex = (float*)malloc(sizeof(float) * 3 * BALL_VERTICES_COUNT);
  ball.elements = (unsigned int*)malloc(sizeof(unsigned int) * BALL_INDICES_COUNT);

  ball.vertex_count = BALL_VERTICES_COUNT;
  ball.elements_count = BALL_INDICES_COUNT;

  memcpy(ball.elements, initial_index, sizeof(initial_index));

  for (p = 0, theta = -M_PI_2; p < BALL_SEGMENTS; p++, theta += UNIT_ANGLE)
  {
    for (m = 0, phi = 0.0f; m < BALL_SEGMENTS; m++, phi += UNIT_ANGLE)
    {
      ball.vertex[vertex++] = cos(theta) * sin(phi) * ball.width;
      ball.vertex[vertex++] = sin(theta) * ball.width;
      ball.vertex[vertex++] = cos(theta) * cos(phi) * ball.width;
    }
  }
  for (int i = 0, j = 0; i < (ball.vertex_count / 2); i++, j += 6) {
      ball.elements[j] = i;
      ball.elements[j + 1] = i + 20;
      ball.elements[j + 2] = ((i + 1) % 20) == 0 ? i + 1 : i + 21;

      ball.elements[j + 3] = i;
      ball.elements[j + 4] = ((i + 1) % 20) == 0 ? i + 1 : i + 21;
      ball.elements[j + 5] = ((i + 1) % 20) == 0 ? i - 19 : i + 1;
  }

  load_identity_matrix(ball.model_matrix);
  ball.model_matrix[14] = -1.0f;
}
void build_circle(PONG_ELEMENT* element, float radius, int segments) {
  element->vertex = (float*)malloc(sizeof(float) * 3 * (segments + 1));
  element->vertex_count = (segments + 1);
  element->elements_count = 0;
  int vertex = 3;
  int p;
  float unit_angle = M_PI_2 / (float)segments;
  float theta;
  element->width = radius;
  element->vertex[0] = 0.0f;
  element->vertex[1] = 0.0f;
  element->vertex[2] = 0.0f;
  for (p = 0, theta = -M_PI_2; p < segments; p++, theta += UNIT_ANGLE) {
        element->vertex[vertex++] = 0.0f;
        element->vertex[vertex++] = sin(theta) * element->width;
        element->vertex[vertex++] = cos(theta) * element->width;
  }
  load_identity_matrix(ball_shadow.model_matrix);
}
void setup_ball_shadow() {
  load_identity_matrix(ball_shadow.model_matrix);
  build_circle(&ball_shadow, BALL_RADIUS, BALL_SEGMENTS);
}

void setup_ball_marks() {
  build_circle(&ball_mark, BALL_RADIUS / 2.0f, BALL_SEGMENTS);
}
void setup_stick_shadows() {

  int elements[] = {0, 1, 2, 0, 2, 3};
  stick_shadow.vertex = (float*)malloc(sizeof(float) * 12);
  stick_shadow.elements = (unsigned int*)malloc(sizeof(unsigned int) * 6);
  stick_shadow.vertex_count = 4;
  stick_shadow.elements_count = 6;
  stick_shadow.width = STICK_WIDTH ;
  stick_shadow.height = STAGE_BLOCK_WIDTH / 50;
  stick_shadow.z = -STAGE_BLOCK_WIDTH / 50;

  stick_shadow.vertex[0] = -stick_shadow.width / 2.0f;
  stick_shadow.vertex[1] = -stick_shadow.height / 2.0f;
  stick_shadow.vertex[2] =  stick_shadow.z;

  stick_shadow.vertex[3] = -stick_shadow.width / 2.0f;
  stick_shadow.vertex[4] = stick_shadow.height / 2.0f;
  stick_shadow.vertex[5] = stick_shadow.z;

  stick_shadow.vertex[6] = stick_shadow.width / 2.0f;
  stick_shadow.vertex[7] = stick_shadow.height / 2.0f;
  stick_shadow.vertex[8] = stick_shadow.z;

  stick_shadow.vertex[9] = stick_shadow.width / 2.0f;
  stick_shadow.vertex[10] = -stick_shadow.height / 2.0f;
  stick_shadow.vertex[11] = stick_shadow.z;
  memcpy(stick_shadow.elements, elements, sizeof(elements));
  load_identity_matrix(stick_shadow.model_matrix);
}

void render_pong_element(PONG_ELEMENT* element) {
  glBindVertexArray(element->vao);
  glUniformMatrix4fv(modelMatrixId, 1, GL_FALSE, element->model_matrix);
  if (element->elements_count > 0) {
    glDrawElements(GL_TRIANGLES, element->elements_count, GL_UNSIGNED_INT, 0);
  } else {
    glDrawArrays(GL_TRIANGLES, 0, element->vertex_count);
  }
  glBindVertexArray(0);
}

void render_shadows(PONG_ELEMENT* stage, PONG_ELEMENT* ball, PONG_ELEMENT* stick) {

    load_identity_matrix(ball_shadow.model_matrix);

    ball_shadow.model_matrix[12] = -stage->width / 2.0f;
    ball_shadow.model_matrix[13] = ball->model_matrix[13];
    ball_shadow.model_matrix[14] = ball->model_matrix[14];
    render_pong_element(&ball_shadow);

    ball_shadow.model_matrix[12] = stage->width / 2.0f;
    ball_shadow.model_matrix[13] = ball->model_matrix[13];
    ball_shadow.model_matrix[14] = ball->model_matrix[14];
    render_pong_element(&ball_shadow);

    ball_shadow.model_matrix[0] = 0.0f;
    ball_shadow.model_matrix[1] = 1.0f;
    ball_shadow.model_matrix[4] = -1.0f;
    ball_shadow.model_matrix[5] = 0.0f;

    ball_shadow.model_matrix[12] = ball->model_matrix[12];
    ball_shadow.model_matrix[13] = -stage->height / 2.0f;
    ball_shadow.model_matrix[14] = ball->model_matrix[14];
    render_pong_element(&ball_shadow);

    ball_shadow.model_matrix[12] = ball->model_matrix[12];
    ball_shadow.model_matrix[13] = stage->height / 2.0f;
    ball_shadow.model_matrix[14] = ball->model_matrix[14];
    render_pong_element(&ball_shadow);

    stick_shadow.model_matrix[0] = 1.0f;
    stick_shadow.model_matrix[1] = 0.0f;
    stick_shadow.model_matrix[4] = 0.0f;
    stick_shadow.model_matrix[5] = 1.0f;

    stick_shadow.model_matrix[12] = stick->model_matrix[12];
    stick_shadow.model_matrix[13] = -stage->height / 2.0f;
    render_pong_element(&stick_shadow);


    stick_shadow.model_matrix[12] = stick->model_matrix[12];
    stick_shadow.model_matrix[13] = stage->height / 2.0f;
    render_pong_element(&stick_shadow);


    stick_shadow.model_matrix[0] = 0.0f;
    stick_shadow.model_matrix[1] = 1.0f;
    stick_shadow.model_matrix[4] = -1.0f;
    stick_shadow.model_matrix[5] = 0.0f;
    stick_shadow.model_matrix[10] = 0.0f;

    stick_shadow.model_matrix[12] = -stage->width / 2.0f;
    stick_shadow.model_matrix[13] = stick->model_matrix[13];
    render_pong_element(&stick_shadow);


    stick_shadow.model_matrix[12] = stage->width / 2.0f;
    stick_shadow.model_matrix[13] = stick->model_matrix[13];
    render_pong_element(&stick_shadow);

}

void render_balls(PONG_ELEMENT* stage) {
  ball_mark.model_matrix[0] = 0.0f;
  ball_mark.model_matrix[1] = 0.0f;
  ball_mark.model_matrix[2] = 1.0f;
  ball_mark.model_matrix[3] = 0.0f;

  ball_mark.model_matrix[4] = 0.0f;
  ball_mark.model_matrix[5] = 1.0f;
  ball_mark.model_matrix[6] = 0.0f;
  ball_mark.model_matrix[7] = 0.0f;

  ball_mark.model_matrix[8] = -1.0f;
  ball_mark.model_matrix[9] = 0.0f;
  ball_mark.model_matrix[10] = 0.0f;
  ball_mark.model_matrix[11] = 0.0f;
  ball_mark.model_matrix[15] = 1.0f;
  ball_mark.model_matrix[13] = -stage->height / 2.0f + 0.05f;

  float gap = ball_mark.width * 3.0f;
  ball_mark.model_matrix[12] = -((float)(balls + 1) * gap) / 2.0f;

  for (int i = 0; i < balls; i++) {
    ball_mark.model_matrix[12] += gap;
    ball_mark.model_matrix[14] = 0.0f;
    render_pong_element(&ball_mark);
  }

}
void render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  render_pong_element(&stage);
  render_pong_element(&enemy_stick);
  render_shadows(&stage, &ball, &player_stick);
  render_balls(&stage);
  render_pong_element(&ball);
  render_pong_element(&player_stick);
  SDL_GL_SwapWindow(window);
}


int ball_in_stick(float ball_x, float ball_y, float ball_width, PONG_ELEMENT* stick) {
  return (
    (((ball_x - ball_width) < (stick->x + stick->width / 2.0f))
    &&
    ((ball_x + ball_width) > (stick->x - stick->width / 2.0f)))
    &&
    (((ball_y - ball_width) < (stick->y + stick->height / 2.0f))
    &&
    ((ball_y + ball_width) > (stick->y - stick->height / 2.0f))));
}
void run_game() {
  int loop = 1;
  unsigned int timeElapsed = 0;
  unsigned int lastTime, currentTime, velocity_sample_time = 0;
  int ball_calc_time_skip = 0;

  float slope;
  float rv;

  int state = 1;

  float to_position[2];
  float opp_vel[2];

  enemy_stick.x = 0.0f;
  enemy_stick.y = 0.0f;
  ball.model_matrix[12] = 0.0f;
  ball.model_matrix[13] = 0.0f;
  ball.model_matrix[14] = player_stick.z - ball.width;

  float ball_x = ball.model_matrix[12];
  float ball_y = ball.model_matrix[13];
  float ball_z = ball.model_matrix[14];

  SDL_Event event;
  SDL_WarpMouseInWindow(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

  currentTime = lastTime = 0;
  velocity_sample_time = lastTime;

  player_stick.xprev = 0.0f;
  player_stick.yprev = 0.0f;

  memcpy(ball_speed_vector, INITIAL_BALL_SPEED_VECTOR, sizeof(INITIAL_BALL_SPEED_VECTOR));

	while (loop)
	{

    currentTime = SDL_GetTicks();

    timeElapsed = currentTime - lastTime;
    lastTime = currentTime;



    if (state == 2) {

      if (ball_speed_vector[2] > 0) {
        ball_speed_vector[2] += 0.00006;
      } else {
        ball_speed_vector[2] -= 0.00006;
      }

      ball_x += ball_speed_vector[0] * (timeElapsed / 1000.0f);
      ball_y += ball_speed_vector[1] * (timeElapsed / 1000.0f);
      ball_z += ball_speed_vector[2] * (timeElapsed / 1000.0f);
    }
    if (state == 2) {

          if ((currentTime - velocity_sample_time) > VELOCITY_SAMPLE_F) {
            player_stick.xprev = player_stick.x;
            player_stick.yprev = player_stick.y;
            velocity_sample_time = currentTime;
          }


      if ((ball_z + ball.width) < enemy_stick.z) {

          if (ball_in_stick(ball_x, ball_y, ball.width, &enemy_stick)) {
              ball_z = enemy_stick.z + ball.width;
              ball_speed_vector[2] *= -1.0f;
              slope = -enemy_stick.y / -enemy_stick.x;
              if (enemy_stick.x >= 0) {
                  rv = -0.3f;
              } else {
                rv = 0.3f;
              }
              SDL_QueueAudio(dev, opponent_pong_sound, opponent_pong_sound_samples * sizeof(float));
          } else {
            state = 4;
            SDL_QueueAudio(dev, player_score_sound, player_score_sound_samples * sizeof(float));
          }
      } else if ((ball_z + ball.width) > player_stick.z) {

          if (ball_in_stick(ball_x, ball_y, ball.width, &player_stick)) {
                ball_z = player_stick.z - ball.width;
                ball_speed_vector[2] *= -1.0f;
                ball_speed_vector[0] += (player_stick.x - player_stick.xprev) / (VELOCITY_SAMPLE_F / 1000.0f);
                ball_speed_vector[1] += (player_stick.y - player_stick.yprev) / (VELOCITY_SAMPLE_F / 1000.0f);
                SDL_QueueAudio(dev, player_pong_sound, player_pong_sound_samples * sizeof(float));
          } else if ((ball_z - ball.width) > -view_matrix[14]) {
            state = 3;
            SDL_QueueAudio(dev, opp_score_sound, opp_score_sound_samples * sizeof(float));
          }
      }

      short wallhit = 0;
      if (ball_x > ((stage.width / 2.0f) - (ball.width))) {
         ball_x = (stage.width / 2.0f) - (ball.width);
         ball_speed_vector[0] *= -1.0f;
         SDL_QueueAudio(dev, wall_hit_sound, wall_hit_sound_samples * sizeof(float));
      }
      else if (ball_x < ((-stage.width / 2.0f) + (ball.width))) {
         ball_x = (-stage.width / 2.0f) + (ball.width);
         ball_speed_vector[0] *= -1.0f;
        SDL_QueueAudio(dev, wall_hit_sound, wall_hit_sound_samples * sizeof(float));
      }

      if (ball_y > ((stage.height / 2) - (ball.width))) {
         ball_y = (stage.height / 2.0f) - (ball.width);
         ball_speed_vector[1] *= -1.0f;
         SDL_QueueAudio(dev, wall_hit_sound, wall_hit_sound_samples * sizeof(float));
      }
      else if (ball_y < ((-stage.height / 2) + (ball.width))) {
         ball_y = (-stage.height / 2.0f) + (ball.width);
         ball_speed_vector[1] *= -1.0f;
        SDL_QueueAudio(dev, wall_hit_sound, wall_hit_sound_samples * sizeof(float));
      }
      if (wallhit) {

        wallhit = 0;
      }

      ball.model_matrix[12] = ball_x;
      ball.model_matrix[13] = ball_y;
      ball.model_matrix[14] = ball_z;

      if (ball_speed_vector[2] > 0.0f && enemy_stick.x < 0.0f) {
        enemy_stick.x += rv * (timeElapsed / 1000.0f);
        enemy_stick.y = enemy_stick.x * slope;
        enemy_stick.model_matrix[12] = enemy_stick.x;
        enemy_stick.model_matrix[13] = enemy_stick.y;

      } else if (ball_speed_vector[2] < 0.0f) {
          if ((currentTime - ball_calc_time_skip) > OPPONENT_SAMPLE_ADJUST) {
            ball_calc_time_skip = currentTime;
            to_position[0] = ball_x - enemy_stick.x;
            to_position[1] = ball_y - enemy_stick.y ;

            opp_vel[0] = (to_position[0] * fabs(ball_speed_vector[2])) /  fabs(enemy_stick.z - ball_z);
            opp_vel[1] = (to_position[1] * fabs(ball_speed_vector[2])) /  fabs(enemy_stick.z - ball_z);

          }
          if (state == 2) {
          enemy_stick.x += (opp_vel[0] * (timeElapsed / 1000.0f));
          enemy_stick.y += (opp_vel[1] * (timeElapsed / 1000.0f));
          enemy_stick.model_matrix[12] = enemy_stick.x;
          enemy_stick.model_matrix[13] = enemy_stick.y;
        }
      }
    }

    render();

    timeElapsed = SDL_GetTicks() - currentTime;

    int t = 16 - timeElapsed;
    if (t > 0) {
      //printf("%i\n", t);
        //SDL_Delay(t);
    }
    if (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        loop = 0;
      } else if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
          case SDLK_ESCAPE:
            loop = 0;
            break;
          default:
            break;
        }
      } else if (event.type == SDL_MOUSEMOTION) {
            player_stick.wx = event.motion.x;
            player_stick.wy = -event.motion.y;
            player_stick.x = (player_stick.wx - (WINDOW_WIDTH >> 1)) / (float)WINDOW_WIDTH;
            player_stick.y = (player_stick.wy + (WINDOW_HEIGHT >> 1)) / (float)WINDOW_HEIGHT;
            player_stick.model_matrix[12] = player_stick.x;
            player_stick.model_matrix[13] = player_stick.y;
      } else if (event.type == SDL_MOUSEBUTTONUP && state == 1) {

          if (ball_in_stick(ball_x, ball_y, ball.width, &player_stick)) {
            SDL_QueueAudio(dev, player_pong_sound, player_pong_sound_samples * sizeof(float));
            state = 2;
          }
      }
    }

	}
}

void free_pong_element(PONG_ELEMENT* element) {
  free(element->vertex);
  if (element->elements_count > 0) {
    free(element->elements);
  }
}

void dispose_game_elements() {
  free_pong_element(&player_stick);
  free_pong_element(&enemy_stick);
  free_pong_element(&ball);
  free_pong_element(&stage);
  free_pong_element(&ball_shadow);
  free_pong_element(&stick_shadow);
  free_pong_element(&ball_mark);
}
void dispose_screen() {
  SDL_GL_DeleteContext(mainContext);
  SDL_DestroyWindow(window);
}

void dispose_game_element_render(PONG_ELEMENT* element) {
  glDeleteBuffers(1, &element->vbo);
  if (element->elements_count > 0) {
    glDeleteBuffers(1, &element->ebo);
  }
  glDeleteVertexArrays(1, &element->vao);
}

void dispose_audio() {
  SDL_CloseAudioDevice(dev);
  free(player_pong_sound);
  free(opponent_pong_sound);
  free(wall_hit_sound);
  free(start_sound);
  free(player_score_sound);
  free(opp_score_sound);
}
void dispose_renderer() {
  glUseProgram(0);
  glDetachShader(program, vertex_shader);
  glDetachShader(program, fragment_shader);
  glDeleteProgram(program);
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
  dispose_game_element_render(&player_stick);
  dispose_game_element_render(&enemy_stick);
  dispose_game_element_render(&ball);
  dispose_game_element_render(&stage);
  dispose_game_element_render(&ball_shadow);
  dispose_game_element_render(&stick_shadow);
  dispose_audio();
}

void cleanup() {
  dispose_screen();
  dispose_renderer();
  dispose_game_elements();
}

float oscillator(OSCILLATOR_TYPE type, float* ang, float incr) {
  float value = 0.0f;
  switch(type) {
    case SIN:
      value = sin(*ang);
      *ang += incr;
      if (*ang >= P_2PI)
        *ang -= P_2PI;
      break;
    case TRIANGLE: {
      float triValue = *ang * M_PI_2;
      *ang += incr;
      if (triValue < 0)
        value = 1.0 + triValue;
      else
        value = 1.0 - triValue;
      if (*ang >= M_PI)
          *ang -= P_2PI;
      }
      break;
    case SAW:
      value = (*ang / M_PI) - 1.0f;
      *ang += incr;
      if (*ang >= P_2PI)
        *ang -= P_2PI;
      break;
    case COS:
      value = cos(*ang);
      *ang += incr;
      if (*ang >= P_2PI)
        *ang -= P_2PI;
      break;
  }
  return value;
}

int synthetize(SYNTH* synthParams, float** out_samples) {
  int state = 0;
  float value;
  float oldValue = 0.0f;
  float volume = synthParams->volume;

  int samples_count = (float)SAMPLE_FREQ * synthParams->totalTime;
  float *samples = (float*)malloc(samples_count * sizeof(float));

  int attackTimeSamples = (synthParams->attackTime * (float)SAMPLE_FREQ);
  int decayTimeSamples = (synthParams->decayTime * (float)SAMPLE_FREQ);
  int releaseTimeSamples = (synthParams->releaseTime * (float)SAMPLE_FREQ);

  int envCount = attackTimeSamples;
  float slope = volume / (float)attackTimeSamples;

  float phaseIncrOsc1 = (P_2PI / (float)SAMPLE_FREQ) * synthParams->oscillator1_freq;
  float phaseIncrOsc2 = (P_2PI / (float)SAMPLE_FREQ) * synthParams->oscillator2_freq;
  float phase1 = 0.0f;
  float phase2 = 0.0f;

  for (int i = 0; i < samples_count; i++) {
    value = oscillator(synthParams->oscillator1_type, &phase1, phaseIncrOsc1);
    if (synthParams->oscillator2_type != NONE) {
      value = value * oscillator(synthParams->oscillator2_type, &phase2, phaseIncrOsc2);
    }
    // filter
    value = (synthParams->filterBeta1 * value) +  (synthParams->filterBeta2 * oldValue);
    oldValue = value;

    // amplitude envelope
    switch (state) {
      case 0:
      if (envCount > 0) {
        volume += slope;
        envCount--;
      } else {
        state = 1;
        envCount = decayTimeSamples;
        slope = (volume - synthParams->decayValue) / (float)decayTimeSamples;
      }
      case 1:
        if (envCount > 0) {
          envCount--;
          volume -= slope;
        } else {
          state = 2;
          envCount = releaseTimeSamples;
          slope = volume / (float)releaseTimeSamples;
        }
        break;
      case 2:
        if (envCount > 0) {
          envCount--;
          volume -= slope;
        } else {
          state = -1;
          volume = 0.0f;
        }
        break;
    }
    samples[i] = volume * value;
  }

  // simple reverb
  float delayTime = synthParams->delayTime;
  float reverbSize = synthParams->reverbSize;
  int delaySamples = (delayTime * (float)SAMPLE_FREQ);
  if (delaySamples > 0) {
    for (int i = 0; i < samples_count - delaySamples; i++) {
    samples[i + delaySamples] += samples[i] * reverbSize;
    }
  }
  *out_samples = samples;
  return samples_count;
}

int setup_sound() {
   if (SDL_Init(SDL_INIT_AUDIO) < 0) {
     fprintf( stderr, "Sound initialization failed: %s\n", SDL_GetError( ) );
     return -1;
   }


   want.freq = SAMPLE_FREQ;
    want.format = AUDIO_F32SYS;
    want.channels = 1;
    want.samples = 2048;
    want.callback = NULL;

    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (dev == 0) {
       fprintf( stderr, "Failed opening audio device: %s\n", SDL_GetError( ) );
       return -1;
    } else if (have.format != want.format) {
       fprintf( stderr, "Failed getting sample format (AUDIO_F32SYS)\n");
       return -1;
    }
    //
    SDL_PauseAudioDevice(dev, 0);
    SYNTH synthParams;

    synthParams.totalTime = 0.1f;
    synthParams.volume = 1.0f;
    synthParams.attackTime = 0.0f;
    synthParams.decayTime = 0.05f;
    synthParams.releaseTime = 0.05f;
    synthParams.decayValue = 0.3f;
    synthParams.filterBeta1 = 0.2f ;
    synthParams.filterBeta2 = 0.3f;
    synthParams.oscillator1_type = TRIANGLE;
    synthParams.oscillator2_type = NONE;
    synthParams.oscillator1_freq = 700.0f;
    synthParams.delayTime = 0.5f;
    synthParams.reverbSize = 0.1f;
    player_pong_sound_samples = synthetize(&synthParams, &player_pong_sound);

    synthParams.oscillator1_freq = 350.0f;
    synthParams.oscillator2_freq = 350.0f;
    opponent_pong_sound_samples = synthetize(&synthParams, &opponent_pong_sound);

    synthParams.oscillator1_freq = 700.0f;
    synthParams.oscillator2_freq = 700.0f;
    start_sound_samples = synthetize(&synthParams, &start_sound);


    synthParams.totalTime = 0.05f;
    synthParams.volume = 0.7f;
    synthParams.attackTime = 0.01f;
    synthParams.decayTime = 0.03f;
    synthParams.releaseTime = 0.01f;
    synthParams.decayValue = 0.7f;
    synthParams.filterBeta1 = 0.6f ;
    synthParams.filterBeta2 = 0.4f;
    synthParams.oscillator1_type = SIN;
    synthParams.oscillator2_type = SIN;
    synthParams.oscillator1_freq = 200.0f;
    synthParams.oscillator2_freq = 400.0f;
    synthParams.delayTime = 0.0f;
    synthParams.reverbSize = 0.0f;
    wall_hit_sound_samples = synthetize(&synthParams, &wall_hit_sound);

    synthParams.totalTime = 1.0f;
    synthParams.volume = 1.0f;
    synthParams.attackTime = 0.2f;
    synthParams.decayTime = 0.8f;
    synthParams.releaseTime = 0.4f;
    synthParams.decayValue = 0.1f;
    synthParams.filterBeta1 = 0.4 ;
    synthParams.filterBeta2 = 0.4;
    synthParams.oscillator1_type = SIN;
    synthParams.oscillator2_type = COS;
    synthParams.oscillator1_freq = 1046.50f;
    synthParams.oscillator2_freq = 2.0f;
    synthParams.delayTime = 0.0f;
    synthParams.reverbSize = 0.0f;
    player_score_sound_samples = synthetize(&synthParams, &player_score_sound);

    synthParams.totalTime = 0.6f;
    synthParams.volume = 1.0f;
    synthParams.attackTime = 0.01f;
    synthParams.decayTime = 0.3f;
    synthParams.releaseTime = 0.3f;
    synthParams.oscillator1_freq = 323.5f;
    synthParams.oscillator2_freq = 30.0f;
    opp_score_sound_samples = synthetize(&synthParams, &opp_score_sound);

    return 0;
}

int setup_text() {
  if(FT_Init_FreeType(&ft)) {
    fprintf(stderr, "Could not init freetype library\n");
    return -1;
  }
  if(FT_New_Face(ft, "./fonts/main.ttf", 0, &face)) {
    fprintf(stderr, "Could not open font\n");
    return -1;
  }

  return 0;
}

int main(int argc, char** argv) {
  if (setup_sound() < 0) {
    exit(1);
  }
  if (setup_screen(WINDOW_WIDTH, WINDOW_HEIGHT) < 0) {
    exit(1);
  }
  if (setup_text() < 0) {
    exit(1);
  }
  setup_stage();
  setup_player_stick();
  setup_enemy_stick();
  setup_ball();
  setup_ball_shadow();
  setup_ball_marks();
  setup_stick_shadows();
  //setup_start_button();
  setup_renderer(WINDOW_WIDTH, WINDOW_HEIGHT);
  run_game();
  cleanup();
}
