#ifndef _UTILS_H_
#define _UTILS_H_

#include "geometry.h"
#include <stdbool.h>

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

extern GAME_STATE gameState, prevGameState;


void mouse_move_player_stick(int mx, int my);
int ball_in_player_stick();
int ball_in_opponent_stick();
int ball_in_stick(float ball_x, float ball_y, float ball_width, PONG_ELEMENT* stick);
int ball_hit_wall(float* outVector, PONG_ELEMENT* stage, PONG_ELEMENT* ball);

void change_state(GAME_STATE state);

bool equals(float, float);

#endif

