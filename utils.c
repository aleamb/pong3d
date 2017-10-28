#include "utils.h"
#include "pong3d.h"

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
			(((ball_x - ball_width) < (stick->x + stick->width / 2.0f))
			 &&
			 ((ball_x + ball_width) > (stick->x - stick->width / 2.0f)))
			&&
			(((ball_y - ball_width) < (stick->y + stick->height / 2.0f))
			 &&
			 ((ball_y + ball_width) > (stick->y - stick->height / 2.0f))));
}

void mouse_move_player_stick(int mx, int my) {

	move_player_stick(
			(mx - (WINDOW_WIDTH >> 1)) / (float)WINDOW_WIDTH,
			//(aspect * h -> w/h * h -> h)
			(my - (WINDOW_HEIGHT >> 1)) / -(float)WINDOW_WIDTH);
}

bool equals(float a, float b) {
	return (bool)(fabs(a - b) <= ball.width);
}

