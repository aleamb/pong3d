#include "tasks.h"

#include "pong3d.h"
#include <stdbool.h>
#include "screens.h"
#include "renderer.h"
#include "sound.h"
#include <time.h>
#include <stdlib.h>
#include <math.h>


float overlay_fadeout;
int  overlay_fadeout_frames;
float overlay_fadeout_alpha;
int player_score = 0;
int opponent_score = 0;
int balls = BALLS;

float ball_speed_vector[3];

bool resetFramesCounter = true;

int ball_frames_dec_factor = FRAMES_DEC_FACTOR;
int ball_decrement = INITIAL_BALL_VELOCITY_DECREMENT;


bool equals(float a, float b) {
	return (bool)(fabs(a - b) <= ball.width);
}

int start_screen_task(int elapsedFrames) {
	sys_show_cursor(1);
	render(0);
	change_state(STARTED);

	return 0;
}

int loading_players_task(int elapsedFrames, int time_delta, int period) {
	if (elapsedFrames == 0) {
		// overlay fading out velocity
		overlay_fadeout_frames = FPS * 0.3f;
		overlay_fadeout = OVERLAY_ALPHA / overlay_fadeout_frames;
		overlay_fadeout_alpha = overlay_fadeout;
	}
	if (elapsedFrames <= overlay_fadeout_frames) {
		renderer_clear_screen();
		render_stage();
		render_fadeout_overlay(overlay_fadeout_alpha);	
		sys_swap_buffers();
		overlay_fadeout_alpha += overlay_fadeout;

	} else {
		play_start_sound();
		player_score = 0;
		opponent_score = 0;
		balls = BALLS;
		change_state(PLAYER_SERVICE);
	}
	return 0;
}

int player_service_task(int elapsedFrames, int pendingEvent, SysEvent* event) {
	if (elapsedFrames == 0) {
		ball_decrement = INITIAL_BALL_VELOCITY_DECREMENT;
		srand(time(NULL));
		ball_speed_vector[0] = ((rand() % 1) ? 1.0 : -1.0) * stage.width / (FPS * (4 + rand() % 1));
		ball_speed_vector[1] = ((rand() % 1) ? 1.0 : -1.0) * stage.height / (FPS * (4 + rand() % 1));
		ball_speed_vector[2] = -stage.large / (FPS - (ball_decrement * ball_frames_dec_factor));
		reset_ball_position();
		reset_player_stick_position();
		reset_opponent_stick_position();
		sys_mouse_center(WINDOW_WIDTH, WINDOW_HEIGHT);
		sys_show_cursor(0);
		balls--;
		render(0);
		return 0;
	}
	if (pendingEvent && event->type == MOUSELBUTTONUP && ball_in_player_stick()) {
		change_state(PLAYER_RETURN);
		play_player_pong_sound();
		return 0;
	}
	return 0;
}



int playing_task(int elapsedFrames, int pendingEvent) {

	float hit_wall_vector[3];
	static float to_position[2] = { 0, 0 };
	static int framesToPosition = 0;
	int resetFrames = 0;
	static bool lookDesviation = false;

	if (ball_hit_wall(hit_wall_vector, &stage, &ball)) {
		ball_speed_vector[0] *= hit_wall_vector[0];
		ball_speed_vector[1] *= hit_wall_vector[1];
		play_wall_hit_sound();
	}
	if (gameState == OPP_RETURN) {
		if (elapsedFrames == 0) {
			to_position[0] = -opponent_stick.x;
			to_position[1] = -opponent_stick.y;
			framesToPosition = 10;
			if (framesToPosition > 0) {
				to_position[0] /= framesToPosition;
				to_position[1] /= framesToPosition;
			}else {
				to_position[0] = 0;
				to_position[1] = 0;
			}
		}
		if (equals(ball.z - ball.width, player_stick.z)) {
			if (ball_in_player_stick()) {
				play_player_pong_sound();
				ball_speed_vector[2] *= -1.0f;
				change_state(PLAYER_RETURN);
				ball.z = player_stick.z - ball.width;
				ball.model_matrix[14] = ball.z;	
				to_position[0] = to_position[1] = 0;
				ball_decrement++;
				int sub_frames = (FPS - ball_frames_dec_factor * ball_decrement);
				if (sub_frames > 0)
					ball_speed_vector[2] = -stage.large / sub_frames;
				player_stick.xprev = player_stick.x;
				player_stick.yprev = player_stick.y;
				player_stick.zprev = player_stick.z;
				lookDesviation = true;

			} else {
				change_state(OPP_WINS);
			}
		}
		if (!equals(opponent_stick.x, 0.0) || !equals(opponent_stick.y, 0.0))
			move_opponent_stick(opponent_stick.x + to_position[0], opponent_stick.y + to_position[1]);	

		resetFrames = 0;
	} else if (gameState == PLAYER_RETURN) {
		if (equals(ball.z + ball.width, opponent_stick.z)) {

			if (ball_in_opponent_stick()) {
				play_opponent_pong_sound();
				ball_speed_vector[2] *= -1.0f;
				ball.z = opponent_stick.z + ball.width;
				ball.model_matrix[14] = ball.z;
				change_state(OPP_RETURN);
				resetFrames = 1;

			} else {
				change_state(PLAYER_WINS);
			}	
		} else {

			if (elapsedFrames == 4) {
				to_position[0] = ball.x - opponent_stick.x;
				to_position[1] = ball.y - opponent_stick.y ;
				framesToPosition = fabs((opponent_stick.z - ball.z) / ball_speed_vector[2]);
				if (framesToPosition > 0) {
					to_position[0] /= framesToPosition;
					to_position[1] /= framesToPosition;
				}else {
					to_position[0] = 0;
					to_position[1] = 0;
				}
				resetFrames = 1;

			}
			if (!equals(opponent_stick.x, ball.x) || !equals(opponent_stick.y, 0.0))
				move_opponent_stick(opponent_stick.x + to_position[0], opponent_stick.y + to_position[1]);	

		}
		if (lookDesviation) {
			ball_speed_vector[0] += (player_stick.x - player_stick.xprev) * (6.0f / FPS);
			ball_speed_vector[1] += (player_stick.y - player_stick.yprev) * (6.0f / FPS);
			lookDesviation = false;
		}

	}
	move_ball(ball.x + ball_speed_vector[0], ball.y + ball_speed_vector[1], ball.z + ball_speed_vector[2]);
	render(0);
	return resetFrames;

}

int opponent_wins_task(int elapsedFrames) {

	if (elapsedFrames == 0) {
		opponent_score++;
		play_opponent_wins_sound();
		render(0);
	} else if (elapsedFrames > 90) {
		if (balls)
			change_state(OPP_SERVICE);
		else
			change_state(FINISHED);
		return 1;
	}
	return 0;

}

int player_wins_task(int elapsedFrames) {

	if (elapsedFrames == 0) {
		player_score++;
		play_player_wins_sound();
		render(0);
	} else if (elapsedFrames > 90) {
		if (balls)
			change_state(PLAYER_SERVICE);
		else
			change_state(FINISHED);
		return 1;
	}
	return 0;

}



int ball_hit_wall(float* outVector, PONG_ELEMENT* stage, PONG_ELEMENT* ball) {
	outVector[0] = 1.0;
	outVector[1] = 1.0;
	if (ball->x >= (stage->width / 2.0f - ball->width)) {
		outVector[0] = -1.0f;
		move_ball(stage->width / 2.0f - ball->width, ball->y, ball->z);
		return 1;
	}
	if (ball->x < (-stage->width / 2.0f + ball->width)) {
		outVector[0] = -1.0f;
		move_ball(-stage->width / 2.0f + ball->width, ball->y, ball->z);
		return 1;

	}
	if (ball->y > (stage->height / 2.0f - ball->width)) {
		outVector[1] = -1.0f;
		move_ball(ball->x, stage->height / 2.0f - ball->width, ball->z);

		return 1;
	}

	if (ball->y < (-stage->height / 2.0f + ball->width)) {
		outVector[1] = -1.0f;
		move_ball(ball->x, -stage->height / 2.0f + ball->width, ball->z);

		return 1;
	}

	return 0;

}
int opponent_service_task(int elapsedFrames) {
	ball_decrement = INITIAL_BALL_VELOCITY_DECREMENT;
	srand(time(NULL));
	ball_speed_vector[0] = ((rand() % 1) ? 1.0 : -1.0) * stage.width / (FPS * (4 + rand() % 1));
	ball_speed_vector[1] = ((rand() % 1) ? 1.0 : -1.0) * stage.height / (FPS * (4 + rand() % 1));
	ball_speed_vector[2] = stage.large / (FPS - (ball_decrement * ball_frames_dec_factor));

	reset_ball_position();
	reset_player_stick_position();
	reset_opponent_stick_position();
	sys_mouse_center(WINDOW_WIDTH, WINDOW_HEIGHT);
	move_opponent_stick(0.0f,0.0f);
	move_ball(0, 0, opponent_stick.z + ball.width);
	change_state(OPP_RETURN);
	balls--;
	return 0;
}

int finished_task(int elapsedFrames) {
	if (elapsedFrames == 0) {
		sys_show_cursor(1);
		render(0);
	}
	return 0;
}

