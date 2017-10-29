#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "geometry.h"
#include <stdbool.h>


#define WINDOW_TITLE "Pong 3D"

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 378

#define STAGE_BLOCKS 8

/*
 segments for ball. The number of vertices would be BALL_SEGMENTS^2
 and angle for each segment 2*PI / BALL_SEGMENTS
 */
#define BALL_SEGMENTS 20
/*
  delay in frames to limit speed of computer stick
*/
#define OPPONENT_SAMPLE_ADJUST 30

// sample frecuency rate for sound synthetizer
#define SAMPLE_RATE 44100

// alpha value for overlay
#define OVERLAY_ALPHA 0.8f

#define INITIAL_BALL_SPEED_VECTOR { -0.1f,  0.05f, -1.3f }

#define STAGE_COLOR { 0.0f, 1.0f, 0.0f, 0.2f };

#define FPS 40

#define BALLS 12

#define FRAMES_DEC_FACTOR 1

#define INITIAL_BALL_VELOCITY_DECREMENT 15

typedef enum {

	STARTING,
	STARTED,
	LOADING_PLAYERS,
	PLAYER_SERVICE,
	PLAYER_RETURN,
	OPP_RETURN,
	OPP_SERVICE,
	PLAYER_WINS,
	OPP_WINS,
	FINISHED,
	EXIT
}GAME_STATE;

extern int balls;
extern int player_score;
extern int opponent_score;
extern GAME_STATE gameState, prevGameState;

void mouse_move_player_stick(int mx, int my);
int ball_in_player_stick();
int ball_in_opponent_stick();
int ball_in_stick(float ball_x, float ball_y, float ball_width, PONG_ELEMENT* stick);
int ball_hit_wall(float* outVector, PONG_ELEMENT* stage, PONG_ELEMENT* ball);

void change_state(GAME_STATE state);


#endif
