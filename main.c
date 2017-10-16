#include <SDL2/SDL.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/glu.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "pong3d.h"
#include "geometry.h"



#define ERRORMSG_MAX_LENGTH 256


SDL_Window* window;
SDL_GLContext mainContext;

PONG_ELEMENT startText;


GLuint text_fragment_shader;


GLuint textTexture;
GLuint text_program;

GLuint projectionMatrixId_text;
GLuint viewMatrixId_text;
GLuint modelMatrixId_text;
i

float ball_speed_vector[3];

const GLchar* fragment_shader_text_source =
"#version 400 core\n \
in vec2 outUV;\n \
out vec4 color;\n \
uniform sampler2D tex;\n \
void main(void) {\n \
    color = vec4(1.0, 1.0, 1.0, texture(tex, outUV).r);\n \
}";


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

typedef enum {
  START,
  LOADING,
  PLAYER_SERVICE,
  PLAYING,
  OPP_SERVICE,
  PLAYER_WINS,
  PLAYER_LOSE,
  FINISHED,
  NORENDER
}GAME_STATE;

GAME_STATE gameState;

FT_Library ft;
FT_Face face;

GLuint startTextVao;
GLuint *startTextVbo;
GLuint *startTextTextures;

GLuint playerTextVao;
GLuint *playerTextVbo;
GLuint *playerTextTextures;

GLuint oppoTextVao;
GLuint *oppoTextVbo;
GLuint *oppoTextTextures;
GLuint globalAlpha;

int player_score;
int opponent_score;
int balls = NUMBER_OF_BALLS;

float overlay_alpha;

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


void render_shadows(PONG_ELEMENT* stage, PONG_ELEMENT* ball, PONG_ELEMENT* stick) {

    load_identity_matrix(ball_shadow.model_matrix);

    ball_shadow.model_matrix[12] = -stage->width / 2.0f + 0.0001;
    ball_shadow.model_matrix[13] = ball->model_matrix[13];
    ball_shadow.model_matrix[14] = ball->model_matrix[14] - 0.011;
    render_pong_element(&ball_shadow);

    ball_shadow.model_matrix[12] = stage->width / 2.0f - 0.0001;
    ball_shadow.model_matrix[13] = ball->model_matrix[13];
    ball_shadow.model_matrix[14] = ball->model_matrix[14] - 0.011;;
    render_pong_element(&ball_shadow);

    ball_shadow.model_matrix[0] = 0.0f;
    ball_shadow.model_matrix[1] = 1.0f;
    ball_shadow.model_matrix[4] = -1.0f;
    ball_shadow.model_matrix[5] = 0.0f;

    ball_shadow.model_matrix[12] = ball->model_matrix[12];
    ball_shadow.model_matrix[13] = -stage->height / 2.0f + 0.0001;
    ball_shadow.model_matrix[14] = ball->model_matrix[14]- 0.016;
    render_pong_element(&ball_shadow);

    ball_shadow.model_matrix[12] = ball->model_matrix[12];
    ball_shadow.model_matrix[13] = stage->height / 2.0f - 0.0001;
    ball_shadow.model_matrix[14] = ball->model_matrix[14] - 0.016;;
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

  for (int i = 1; i < balls; i++) {
    ball_mark.model_matrix[12] += gap;
    ball_mark.model_matrix[14] = 0.0f;
    render_pong_element(&ball_mark);
  }

}
void render_text(PONG_ELEMENT *element, const char *str, float px, float py, float scale, int size) {
  const char *p;

  element->vertexType = GL_TRIANGLE_STRIP;
  element->vertex_count = 4;
  element->elements_count = 0;
  element->model_matrix[0] = scale;
  element->model_matrix[5] = -scale;
  element->model_matrix[10] = scale;
  element->model_matrix[12] = px - scale * (strlen(str) >> 1);
  element->model_matrix[13] = py;
  float x = 0.0f;
  GLint current_program;
  glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);

 glUseProgram(text_program);

  glGenVertexArrays(1, &element->vao);
  glGenBuffers(1, &element->vbo);
  glGenTextures(1, &element->texture);

  glBindVertexArray(element->vao);
  glBindBuffer(GL_ARRAY_BUFFER, element->vbo);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (char*)NULL + (sizeof(float) * 4));
  glEnableVertexAttribArray(3);
  glUniformMatrix4fv(modelMatrixId_text, 1, GL_FALSE, element->model_matrix);
  FT_Set_Pixel_Sizes(face, 0, size);
  for(p = str; *p; p++) {
    if(FT_Load_Char(face, *p, FT_LOAD_RENDER))
        continue;
    FT_GlyphSlot g = face->glyph;

        GLfloat box[] = {
            1.0 + x, 1.0, 0.2, 1.0, 1, 1,
            1.0 + x, 0.0, 0.2, 1.0, 1, 0,
            0   + x, 1.0, 0.2, 1.0, 0, 1,
            0   + x, 0,   0.2, 1.0, 0, 0};

      x += 1.0;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, element->texture);

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
  glUseProgram(current_program);
  glBindVertexArray(0);
  glDeleteBuffers(1, &element->vbo);
  glDeleteVertexArrays(1, &element->vao);
  glDeleteTextures(1, &element->texture);
}
void render_stage() {
  glUniform1i(glGetUniformLocation(program, "stageWireframe"), 1);
  glPolygonMode(GL_FRONT, GL_LINE);
  render_pong_element(&stage);
  glUniform1i(glGetUniformLocation(program, "stageWireframe"), 0);
  glPolygonMode(GL_FRONT, GL_FILL);
  render_pong_element(&stage);
}
void render_start_screen() {
  render_stage();
  render_pong_element(&overlay);
  render_text(&startText, "Click on screen to begin", 0.0f, 0.0f, 0.02, 48);
}
void render_sticks() {
  render_pong_element(&player_stick);
  render_pong_element(&enemy_stick);

  render_pong_element(&player_stick);
}
void render_scores(PONG_ELEMENT* stage) {
  char text[16];
  sprintf(text, "YOU: %d", player_score);
  render_text(&startText, text, -stage->width / 2.0f + 0.1f, -stage->height / 2.0f + 0.05, 0.02, 48);
  sprintf(text, "Computer: %d", opponent_score);
  render_text(&startText, text, stage->width / 2.0f - 0.15f, -stage->height / 2.0f + 0.05, 0.02, 48);
}
void render_player_lose_screen(int frames) {
  if (frames == 0) {
    SDL_QueueAudio(dev, opp_score_sound, opp_score_sound_samples * sizeof(float));
  } else if (frames < 90) {
      render_stage();
      render_pong_element(&overlay);
      render_text(&startText, "Computer wins", 0, 0, 0.02, 48);
  } else {
    gameState = OPP_SERVICE;
    opponent_score++;
    balls--;
    if (balls == 0) {
        SDL_ShowCursor(SDL_TRUE);
      gameState = FINISHED;
    }
  }
}
void opponent_service(int frames) {

  enemy_stick.x = 0.0f;
  enemy_stick.y = 0.0f;
  enemy_stick.model_matrix[12] = 0.0f;
  enemy_stick.model_matrix[13] = 0.0f;
  ball.model_matrix[12] = 0.0f;
  ball.model_matrix[13] = 0.0f;
  ball.model_matrix[14] = -1.4f;


  memcpy(ball_speed_vector, INITIAL_BALL_SPEED_VECTOR, sizeof(ball_speed_vector));
  ball_speed_vector[2] *= -1.0f;
  gameState = PLAYING;
  SDL_WarpMouseInWindow(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
  balls--;
  if (balls == 0) {
      SDL_ShowCursor(SDL_TRUE);
    gameState = FINISHED;
  }

}
void render_player_wins(int frames) {
  if (frames == 0) {
      player_score++;
    SDL_QueueAudio(dev, player_score_sound, player_score_sound_samples * sizeof(float));
  } else if (frames < 90) {
      render_stage();
      render_pong_element(&overlay);
      render_text(&startText, "You wins", 0, 0, 0.02, 48);
  } else {

    enemy_stick.x = 0.0f;
    enemy_stick.y = 0.0f;
    enemy_stick.model_matrix[12] = 0.0f;
    enemy_stick.model_matrix[13] = 0.0f;
    ball.model_matrix[12] = 0.0f;
    ball.model_matrix[13] = 0.0f;
    ball.model_matrix[14] = player_stick.z - ball.width;
    memcpy(ball_speed_vector, INITIAL_BALL_SPEED_VECTOR, sizeof(ball_speed_vector));
    gameState = PLAYER_SERVICE;
    SDL_WarpMouseInWindow(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
  }
}
void render_finish_screen(int frames) {
  render_stage();
  render_pong_element(&overlay);
  render_text(&startText, "Click on screen to begin", 0.0f, 0.0f, 0.02, 48);
  render_scores(&stage);
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

void task_prepare_players(int frames, float timeElapsed) {
    if (frames <= 20) {
        glUniform1f(globalAlpha, 0.0);
        render_stage();
        glUniform1f(globalAlpha,  OVERLAY_ALPHA / 20.0f * frames);
        render_pong_element(&overlay);
    } else if (frames == 21) {
        glUniform1f(globalAlpha, 0.0);
        render_stage();
        render_pong_element(&ball);
        SDL_QueueAudio(dev, player_pong_sound, player_pong_sound_samples * sizeof(float));
    } else {
      enemy_stick.x = 0.0f;
      enemy_stick.y = 0.0f;
      ball.model_matrix[12] = 0.0f;
      ball.model_matrix[13] = 0.0f;
      ball.model_matrix[14] = player_stick.z - ball.width;
      player_score = 0;
      opponent_score = 0;
      balls = NUMBER_OF_BALLS;
    SDL_ShowCursor(SDL_FALSE);
      gameState = PLAYER_SERVICE;
    }
}
void render(int frames, float delta) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if (gameState == START) {
      render_start_screen();
      SDL_GL_SwapWindow(window);
  } else if (gameState == LOADING) {
      task_prepare_players(frames, delta);
      SDL_GL_SwapWindow(window);
  } else if (gameState == PLAYER_LOSE) {
      render_player_lose_screen(frames);
      SDL_GL_SwapWindow(window);
  } else if (gameState == OPP_SERVICE) {
      opponent_service(frames);
  } else if (gameState == PLAYER_WINS) {
      render_player_wins(frames);
      SDL_GL_SwapWindow(window);
  }
  else if (gameState == FINISHED) {
     render_finish_screen(frames);
     SDL_GL_SwapWindow(window);
 } else {
      render_stage();
      render_shadows(&stage, &ball, &player_stick);
      render_balls(&stage);
      render_pong_element(&ball);
      render_sticks();
      render_scores(&stage);

      SDL_GL_SwapWindow(window);
  }
}
void run_game() {


  int loop = 1;
  unsigned int timeElapsed = 0;
  unsigned int lastTime, currentTime, velocity_sample_time = 0;
  int ball_calc_time_skip = 0;

  float slope;
  float rv;

  int frames = 0;

  float to_position[2];
  float opp_vel[2];

  SDL_Event event;
  SDL_WarpMouseInWindow(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

  currentTime = lastTime = 0;
  velocity_sample_time = lastTime;

  memcpy(ball_speed_vector, INITIAL_BALL_SPEED_VECTOR, sizeof(INITIAL_BALL_SPEED_VECTOR));

  float ball_x = ball.model_matrix[12];
  float ball_y = ball.model_matrix[13];
  float ball_z = ball.model_matrix[14];

	while (loop)
	{
    currentTime = SDL_GetTicks();

    timeElapsed = currentTime - lastTime;
    lastTime = currentTime;


    if (gameState == PLAYING || gameState == PLAYER_SERVICE) {

      if (ball_speed_vector[2] > 0) {
        ball_speed_vector[2] += 0.0006;
      } else {
        ball_speed_vector[2] -= 0.0006;
      }

      ball_x = ball.model_matrix[12] + ball_speed_vector[0] * (timeElapsed / 1000.0f);
      ball_y = ball.model_matrix[13] + ball_speed_vector[1] * (timeElapsed / 1000.0f);
      ball_z = ball.model_matrix[14] + ball_speed_vector[2] * (timeElapsed / 1000.0f);
    }

    if (gameState == PLAYING) {

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
            gameState = PLAYER_WINS;
            frames = 0;
          }
      } else if ((ball_z + ball.width) > player_stick.z) {

          if (ball_in_stick(ball_x, ball_y, ball.width, &player_stick)) {
                ball_z = player_stick.z - ball.width;
                ball_speed_vector[2] *= -1.0f;
                ball_speed_vector[0] += (player_stick.x - player_stick.xprev) / (VELOCITY_SAMPLE_F / 1000.0f);
                ball_speed_vector[1] += (player_stick.y - player_stick.yprev) / (VELOCITY_SAMPLE_F / 1000.0f);
                SDL_QueueAudio(dev, player_pong_sound, player_pong_sound_samples * sizeof(float));

          } else if ((ball_z - ball.width) < player_stick.z) {
              gameState = PLAYER_LOSE;
              frames = 0;
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
            if (gameState == PLAYING) {
            enemy_stick.x += (opp_vel[0] * (timeElapsed / 1000.0f));
            enemy_stick.y += (opp_vel[1] * (timeElapsed / 1000.0f));
            enemy_stick.model_matrix[12] = enemy_stick.x;
            enemy_stick.model_matrix[13] = enemy_stick.y;
          }
        }
      }


    render(frames++, timeElapsed / 1000.0f);

    timeElapsed = SDL_GetTicks() - currentTime;

    int t = 17 - timeElapsed;
    if (SDL_WaitEventTimeout(&event, t)) {
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
          if (gameState != LOADING && gameState != FINISHED) {
            player_stick.x = (event.motion.x - (WINDOW_WIDTH >> 1)) / (float)WINDOW_WIDTH;
            //(aspect * h -> w/h * h -> h)
            player_stick.y = (event.motion.y - (WINDOW_HEIGHT >> 1)) / -(float)WINDOW_WIDTH;
            player_stick.model_matrix[12] = player_stick.x;
            player_stick.model_matrix[13] = player_stick.y;
          }
      } else if (event.type == SDL_MOUSEBUTTONUP) {
          if (gameState == START || gameState == FINISHED) {
            frames = 0;
            gameState = LOADING;
            //task_prepare_players((timeElapsed / 1000.0f));
          } else if (gameState == PLAYER_SERVICE && ball_in_stick(ball_x, ball_y, ball.width, &player_stick)) {
            SDL_QueueAudio(dev, player_pong_sound, player_pong_sound_samples * sizeof(float));
            gameState = PLAYING;
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
    case NONE:
      value = 0;
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

  glUniform1i(glGetUniformLocation(program, "tex"), 0);

  // generate start screen text
  load_identity_matrix(startText.model_matrix);
  return 0;
}

void setup_game_data() {

}


int main(int argc, char** argv) {
  if (setup_sound() < 0) {
    exit(1);
  }
  if (setup_screen(WINDOW_WIDTH, WINDOW_HEIGHT) < 0) {
    exit(1);
  }
  setup_game_data();
  setup_stage();
  setup_overlay();
  setup_player_stick();
  setup_enemy_stick();
  setup_ball();
  setup_ball_shadow();
  setup_ball_marks();
  setup_stick_shadows();
  setup_renderer(WINDOW_WIDTH, WINDOW_HEIGHT);
  if (setup_text() < 0) {
    exit(1);
  }
  run_game();
  cleanup();
}
