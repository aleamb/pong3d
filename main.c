#include <SDL2/SDL.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/gl.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define STAGE_BLOCKS 5
#define BALL_SEGMENTS 20

#define STAGE_VERTICES_COUNT (4 * STAGE_BLOCKS + 4)
#define STAGE_INDICES_COUNT (STAGE_BLOCKS * 8)
#define BALL_VERTICES_COUNT (BALL_SEGMENTS * BALL_SEGMENTS)
#define BALL_INDICES_COUNT (BALL_SEGMENTS * BALL_SEGMENTS * 6 + 6)
#define UNIT_ANGLE 6.283185307f / BALL_SEGMENTS


SDL_Window* window;
SDL_GLContext mainContext;

typedef struct {
  float* vertex;
  int* elements;
  int vertex_count;
  int elements_count;
  float x;
  float y;
  float z;
  GLuint vao;
  GLuint vbo;
  GLuint ebo;
}PONG_ELEMENT;

PONG_ELEMENT player_stick;
PONG_ELEMENT enemy_stick;
PONG_ELEMENT ball;
PONG_ELEMENT stage;

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
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, element->elements_count * sizeof(int), element->elements, GL_STATIC_DRAW);
  glBindVertexArray(0);
}

int setup_renderer(int width, int height) {

  GLenum err = glewInit();
  if (err != GLEW_OK) {
    fprintf(stderr, "OpenGL error: %s\n", glewGetErrorString(err));
    return -1;
  }
  glViewport(0, 0, width, height);
  glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
  glDisable(GL_CULL_FACE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  create_vao(&player_stick);
  create_vao(&enemy_stick);
  create_vao(&ball);
  create_vao(&stage);

  glCreateShader(GL_VERTEX_SHADER);

  return 0;
}

void setup_stick(PONG_ELEMENT* stick) {
  stick->vertex = (float*)malloc(sizeof(float) * 12);
  stick->vertex_count = 4;
  stick->elements = (int*)malloc(sizeof(int) * 6);
  stick->elements_count = 6;
  stick->vertex[0] = -0.5f;
  stick->vertex[1] = -0.5f;
  stick->vertex[2] = 0.0f;

  stick->vertex[3] = -0.5f;
  stick->vertex[4] = 0.5f;
  stick->vertex[5] = 0.0f;

  stick->vertex[6] = 0.5f;
  stick->vertex[7] = 0.5f;
  stick->vertex[8] = 0.0f;

  stick->vertex[9] = 0.5f;
  stick->vertex[10] = -0.5f;
  stick->vertex[11] = 0.0f;
}

void setup_player_stick() {
  player_stick.x = 0.0f;
  player_stick.y = 0.0f;
  player_stick.z = 0.0f;
  setup_stick(&player_stick);
}

void setup_enemy_stick() {
  enemy_stick.x = 0.0f;
  enemy_stick.y = 0.0f;
  enemy_stick.z = -1.0f;
  setup_stick(&enemy_stick);
}

void setup_stage() {
  int vertex = 0;
  int index = 6;
  int initial_index[] = { 0, 4, 5, 5, 1, 0 };

  stage.vertex = (float*)malloc(sizeof(float) * 3 * STAGE_VERTICES_COUNT);
  stage.elements = (int*)malloc(sizeof(int) * STAGE_INDICES_COUNT);

  stage.vertex_count = STAGE_VERTICES_COUNT;
  stage.elements_count = STAGE_INDICES_COUNT;

  memcpy(stage.elements, initial_index, sizeof(initial_index));

  for (int i = 0; i <= STAGE_BLOCKS; i++) {
    stage.vertex[vertex++] = -0.5f;
    stage.vertex[vertex++] = 0.5f;
    stage.vertex[vertex++] = -1.0f * i;

    stage.vertex[vertex++] = 0.5f;
    stage.vertex[vertex++] = 0.5f;
    stage.vertex[vertex++] = -1.0f * i;

    stage.vertex[vertex++] = 0.5f;
    stage.vertex[vertex++] = -0.5f;
    stage.vertex[vertex++] = -1.0f * i;

    stage.vertex[vertex++] = -0.5f;
    stage.vertex[vertex++] = -0.5f;
    stage.vertex[vertex++] = -1.0f * i;

    stage.elements[index++] = initial_index[0]++;
    stage.elements[index++] = initial_index[1]++;
    stage.elements[index++] = initial_index[2]++;
    stage.elements[index++] = initial_index[3]++;
    stage.elements[index++] = initial_index[4]++;
    stage.elements[index++] = initial_index[5]++;
  }
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
}

void run_game() {
  int loop = 1;

	while (loop)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				loop = 0;

			if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					loop = 0;
					break;
				default:
					break;
				}
			}
		}
  SDL_GL_SwapWindow(window);
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
    //glDisableVertexAttribArray(0);
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
