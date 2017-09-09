#include <SDL2/SDL.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>


#define ERRORMSG_MAX_LENGTH 256

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 378

#define STAGE_BLOCKS 7
#define BALL_SEGMENTS 20

#define STAGE_VERTICES_COUNT (4 * STAGE_BLOCKS + 4)
#define STAGE_INDICES_COUNT (STAGE_BLOCKS * 24)
#define BALL_VERTICES_COUNT (BALL_SEGMENTS * BALL_SEGMENTS)
#define BALL_INDICES_COUNT (BALL_SEGMENTS * BALL_SEGMENTS * 6 + 6)
#define UNIT_ANGLE 6.283185307f / BALL_SEGMENTS

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
  GLuint vao;
  GLuint vbo;
  GLuint ebo;
  float width;
  float height;
  float model_matrix[16];
}PONG_ELEMENT;

PONG_ELEMENT player_stick;
PONG_ELEMENT enemy_stick;
PONG_ELEMENT ball;
PONG_ELEMENT stage;

GLuint vertex_shader;
GLuint fragment_shader;
GLuint program;

GLchar errormsg[ERRORMSG_MAX_LENGTH];

const GLchar* vertex_shader_source = "#version 430 core\n \
in vec3 in_Position;\n \
uniform mat4 projectionMatrix;\n \
uniform mat4 viewMatrix;\n \
uniform mat4 modelMatrix;\n \
void main(void) {\n \
  vec4 pos = modelMatrix * vec4(in_Position.x, in_Position.y , in_Position.z, 1.0);\n \
  gl_Position = viewMatrix * projectionMatrix * pos;\n \
}";

const GLchar* fragment_shader_source = "#version 430 core\n \
precision highp float;\n \
in vec4 ex_Color;\n \
void main(void) {\n \
  gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);\n \
}";

float projection_matrix[16];
float view_matrix[16];

GLuint projectionMatrixId;
GLuint viewMatrixId;
GLuint modelMatrixId;

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

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  return 0;
}

void normalize(float* mesh, int size) {
  int _size = size * 3;
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

  glGenBuffers(1, &element->ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, element->elements_count * sizeof(unsigned int), element->elements, GL_STATIC_DRAW);

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
    glBindAttribLocation(program_id, 0, "in_Position");
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

  memset(out, 0, sizeof(projection_matrix));
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
  //glEnable(GL_PROGRAM_POINT_SIZE);
  //glPointSize(3.0f);
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

  glUseProgram(program);

  create_vao(&player_stick);
  create_vao(&enemy_stick);
  create_vao(&ball);
  create_vao(&stage);

  load_identity_matrix(view_matrix);
  create_projection_matrix(60.0f, (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.0f, 100.0f, projection_matrix);
  projectionMatrixId = glGetUniformLocation(program, "projectionMatrix");
  viewMatrixId = glGetUniformLocation(program, "viewMatrix");
  modelMatrixId = glGetUniformLocation(program, "modelMatrix");
  glUniformMatrix4fv(projectionMatrixId, 1, GL_FALSE, projection_matrix);
  glUniformMatrix4fv(viewMatrixId, 1, GL_FALSE, view_matrix);

  return 0;
}


void setup_stick(PONG_ELEMENT* stick) {

  float size_x = 1.0f;
  float size_y = 1.0f;
  stick->vertex = (float*)malloc(sizeof(float) * 12);
  stick->vertex_count = 4;
  stick->elements = (int*)malloc(sizeof(int) * 6);

  stick->elements_count = 6;
  stick->vertex[0] = -size_x;
  stick->vertex[1] = -size_y;
  stick->vertex[2] = 0.0f;

  stick->vertex[3] = -size_x;
  stick->vertex[4] = size_y;
  stick->vertex[5] = 0.0f;

  stick->vertex[6] = size_x;
  stick->vertex[7] = size_y;
  stick->vertex[8] = 0.0f;

  stick->vertex[9] = size_x;
  stick->vertex[10] = -size_y;
  stick->vertex[11] = 0.0f;

  stick->elements[0] = 0;
  stick->elements[1] = 1;
  stick->elements[2] = 2;
  stick->elements[3] = 2;
  stick->elements[4] = 3;
  stick->elements[5] = 0;

  load_identity_matrix(stick->model_matrix);

}

void setup_player_stick() {
  player_stick.x = 0.0f;
  player_stick.y = 0.0f;
  player_stick.z = 0.0f;
  setup_stick(&player_stick);
  player_stick.model_matrix[14] = -2.0f;
}

void setup_enemy_stick() {
  enemy_stick.x = 0.0f;
  enemy_stick.y = 0.0f;
  enemy_stick.z = -1.0f;
  setup_stick(&enemy_stick);
}

void setup_stage() {

  int vertex = 0;
  float aspect = (float)WINDOW_WIDTH / WINDOW_HEIGHT;
  stage.vertex = (float*)malloc(sizeof(float) * 3 * STAGE_VERTICES_COUNT);
  stage.elements = (int*)malloc(sizeof(int) * STAGE_INDICES_COUNT);

  stage.vertex_count = STAGE_VERTICES_COUNT;
  stage.elements_count = STAGE_INDICES_COUNT;

  for (int i = 0; i <= STAGE_BLOCKS; i++) {
    stage.vertex[vertex++] = -1.0f * aspect;
    stage.vertex[vertex++] = 1.0f;
    stage.vertex[vertex++] = (-0.5f * (float)i);

    stage.vertex[vertex++] = 1.0f * aspect;
    stage.vertex[vertex++] = 1.0f;
    stage.vertex[vertex++] = (-0.5f * (float)i);

    stage.vertex[vertex++] = 1.0f * aspect;
    stage.vertex[vertex++] = -1.0f ;
    stage.vertex[vertex++] = (-0.5f * (float)i);

    stage.vertex[vertex++] = -1.0f * aspect;
    stage.vertex[vertex++] = -1.0f ;
    stage.vertex[vertex++] = (-0.5f * (float)i);
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
  stage.model_matrix[14] = -1.7f;
}

void setup_ball() {

  int initial_index[] = {0, 5, 6, 6, 1, 0};
  float theta;
  float phi;
  int vertex = 0;
  int index = 6;

  ball.vertex = (float*)malloc(sizeof(float) * 3 * BALL_VERTICES_COUNT);
  ball.elements = (int*)malloc(sizeof(int) * BALL_INDICES_COUNT);

  ball.vertex_count = BALL_VERTICES_COUNT;
  ball.elements_count = BALL_INDICES_COUNT;

  memcpy(ball.elements, initial_index, sizeof(initial_index));

  for (int p = 0, theta = -M_PI_2; p < BALL_SEGMENTS; p++, theta += UNIT_ANGLE)
  {
    for (int m = 0, phi = 0.0f; m < BALL_SEGMENTS; m++, phi += UNIT_ANGLE)
    {
      ball.vertex[vertex++] = cos(theta) * sin(phi);
      ball.vertex[vertex++] = sin(theta);
      ball.vertex[vertex++] = cos(theta) * cos(phi);

      ball.elements[index++] = initial_index[0]++;
      ball.elements[index++] = initial_index[1]++;
      ball.elements[index++] = initial_index[2]++;
      ball.elements[index++] = initial_index[3]++;
      ball.elements[index++] = initial_index[4]++;
      ball.elements[index++] = initial_index[5]++;
    }
  }
  load_identity_matrix(ball.model_matrix);
}
void render_pong_element(PONG_ELEMENT* element) {
  glBindVertexArray(element->vao);
  glUniformMatrix4fv(modelMatrixId, 1, GL_FALSE, element->model_matrix);
  glDrawElements(GL_TRIANGLES, element->elements_count, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}
void render() {

  render_pong_element(&stage);
  render_pong_element(&player_stick);
  //render_pong_element(enemy_stick);
  //render_pong_element(ball);


  SDL_GL_SwapWindow(window);
}
void run_game() {
  int loop = 1;

	while (loop)
	{
		SDL_Event event;
		SDL_PollEvent(&event);

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
		}
    render();
	}
}

void free_pong_element(PONG_ELEMENT* element) {
  free(element->vertex);
  free(element->elements);
}

void dispose_game_elements() {
  free_pong_element(&player_stick);
  free_pong_element(&enemy_stick);
  free_pong_element(&ball);
  free_pong_element(&stage);
}
void dispose_screen() {
  SDL_GL_DeleteContext(mainContext);
  SDL_DestroyWindow(window);
}

void dispose_game_element_render(PONG_ELEMENT* element) {
  glDeleteBuffers(1, &element->vbo);
  glDeleteBuffers(1, &element->ebo);
  glDeleteVertexArrays(1, &element->vao);
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
}

void cleanup() {
  dispose_screen();
  dispose_renderer();
  dispose_game_elements();
}
int main(int argc, char** argv[]) {

  if (setup_screen(WINDOW_WIDTH, WINDOW_HEIGHT) < 0) {
    exit(1);
  }
  setup_stage();
  setup_player_stick();
  setup_enemy_stick();
  setup_ball();
  // setup_game();
  setup_renderer(WINDOW_WIDTH, WINDOW_HEIGHT);
  run_game();
  cleanup();
}
