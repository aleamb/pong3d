/**
  @file pong3d.c
  @author Alejandro Ambroa
  @date 1 Oct 2017
  @brief General data and utils for game.
 */



#include "pong3d.h"
#include "geometry.h"

int balls;
int player_score;
int opponent_score;
GAME_STATE gameState, prevGameState;

void change_state(GAME_STATE state) {
	prevGameState = gameState;
	gameState = state;
}

int ball_in_player_stick() {
	return ball_in_stick(ball.x, ball.y, ball.width, &player_stick);
}

int ball_in_opponent_stick() {
	return ball_in_stick(ball.x, ball.y, ball.width, &opponent_stick);
}


int ball_in_stick(float ball_x, float ball_y, float ball_width, PONG_ELEMENT* stick) {
	return (
			(((ball_x - ball_width) < (stick->x + stick->width2))
			 &&
			 ((ball_x + ball_width) > (stick->x - stick->width2)))
			&&
			(((ball_y - ball_width) < (stick->y + stick->height2))
			 &&
			 ((ball_y + ball_width) > (stick->y - stick->height2))));
}

void mouse_move_player_stick(int mx, int my) {

	move_player_stick(
			(mx - (WINDOW_WIDTH >> 1)) / (float)WINDOW_WIDTH,
			//(aspet * h -> w/h * h -> h)
			(my - (WINDOW_HEIGHT >> 1)) / -(float)WINDOW_WIDTH);
}
