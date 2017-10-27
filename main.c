#include "msys.h"
#include "sound.h"
#include "geometry.h"
#include "renderer.h"
#include "text.h"
#include "screens.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 378
#define FPS 40
#define BALLS 12
#define SAMPLE_RATE 44100
#define FRAMES_DEC_FACTOR 1
#define INITIAL_BALL_VELOCITY_DECREMENT 10


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

GAME_STATE gameState, prevGameState;

int player_score;
int opponent_score;
int balls;


void run_game();
void init_game();
void render(int);
int process_state(int, int, int, int, SysEvent* event);

int start_screen_task(int);
int loading_players_task(int, int, int);
int process_events_task(SysEvent* event, int, int);
int player_service_task(int, int, SysEvent*);
int opponent_service_task(int);
int playing_task(int, int);
int opponent_wins_task(int);
int player_wins_task(int);
int finished_task(int);

void mouse_move_player_stick(int mx, int my);
int ball_in_player_stick();
int ball_in_opponent_stick();
int ball_in_stick(float ball_x, float ball_y, float ball_width, PONG_ELEMENT* stick);
int ball_hit_wall(float* outVector, PONG_ELEMENT* stage, PONG_ELEMENT* ball);


void change_state(GAME_STATE state);


void cleanup();


float overlay_fadeout;
int  overlay_fadeout_frames;
float overlay_fadeout_alpha;


float ball_speed_vector[3];

int scaled_dimensions[3];
int vel;

bool resetFramesCounter = true;

int ball_frames_dec_factor = FRAMES_DEC_FACTOR;
int ball_decrement = INITIAL_BALL_VELOCITY_DECREMENT;

int main(int argc, char** argv) {

	if (sys_init_video(WINDOW_WIDTH, WINDOW_HEIGHT) < 0) {
		cleanup();
		exit(1);
	}
	if (init_sound(SAMPLE_RATE) < 0) {
		cleanup();
		exit(1);
	}
	if (init_renderer(WINDOW_WIDTH, WINDOW_HEIGHT) < 0) {
		cleanup();
		exit(1);
	}
	if (init_text_renderer() < 0) {
		cleanup();
		exit(1);
	}
	create_elements(WINDOW_WIDTH, WINDOW_HEIGHT);

	run_game();
	cleanup();	
	return 0;
}

void cleanup() {
	dispose_elements();
	dispose_renderer();
	dispose_text_renderer();
	sys_dispose_video();
	dispose_sound();
	sys_quit();
}


void init_game() {
	balls = BALLS;
	reset_player_stick_position();
	reset_opponent_stick_position();
	reset_ball_position();
	player_score = 0;
	opponent_score = 0;
	sys_show_cursor(0);
	sys_mouse_center(WINDOW_WIDTH, WINDOW_HEIGHT);
}

void run_game() {

	int elapsedFrames = 0;
	unsigned int startTime, elapsedTime;
	int period = ((1.0f / FPS) * 1000.0f) ;
	int pendingEvent = 0;
	SysEvent event;
	bool firstLoop = true;
	int reset_frames_counter = 0;
	int wait_time = 0;
	int time_last_frame = startTime = sys_get_ticks();
	GAME_STATE currentState = STARTING;
	change_state(STARTING);

	while (gameState != EXIT) {
		startTime = sys_get_ticks();
		if (pendingEvent) {
			process_events_task(&event, elapsedFrames, period);
				
		}
		if (firstLoop || (startTime - time_last_frame) >= period) {
			if (currentState != gameState || reset_frames_counter) {
				elapsedFrames = 0;	
				currentState = gameState;
			}
			reset_frames_counter = process_state(elapsedFrames, startTime - time_last_frame, period, pendingEvent, &event);
			elapsedFrames++;

			time_last_frame = sys_get_ticks();

			elapsedTime = time_last_frame - startTime;
			wait_time = period - elapsedTime;	
			firstLoop = false;
		} else {
			wait_time -= (sys_get_ticks() - startTime); 
		}
		pendingEvent = sys_wait(&event, wait_time);


	}

}

void change_state(GAME_STATE state) {
	prevGameState = gameState;
	gameState = state;
}

int process_state(int elapsedFrames, int time_delta, int period, int pendingEvent, SysEvent* event) {
	int reset_frames = 1;
	switch (gameState) {

		case STARTING:
			reset_frames = start_screen_task(elapsedFrames);
			break;
		case LOADING_PLAYERS:
			reset_frames = loading_players_task(elapsedFrames, time_delta, period);
			break;
		case PLAYER_SERVICE:
			reset_frames = player_service_task(elapsedFrames, pendingEvent, event);
			break;
		case PLAYER_RETURN:
		case OPP_RETURN:
			reset_frames = playing_task(elapsedFrames, pendingEvent);
			break;
		case PLAYER_WINS:
			reset_frames = player_wins_task(elapsedFrames);
			break;
		case OPP_WINS:
			reset_frames = opponent_wins_task(elapsedFrames);
			break;
		case OPP_SERVICE:
			reset_frames = opponent_service_task(elapsedFrames);
		case FINISHED:
			reset_frames = finished_task(elapsedFrames);
			break;
		case EXIT:
		case STARTED:
			break;

	}
	return reset_frames;
}

void render(int time_delta) {
	renderer_clear_screen();
	switch (gameState) {
		case STARTING:
			render_start_screen();
		case LOADING_PLAYERS:
			render_stage();
			render_fadeout_overlay(overlay_fadeout_alpha);
			break;
		case PLAYER_SERVICE:
		case PLAYER_RETURN:
		case OPP_RETURN:
			render_main_screen(balls, player_score, opponent_score);
			break;
		case PLAYER_WINS:
			render_player_wins_screen();
			break;

		case OPP_WINS:
			render_opp_wins_screen();
				break;
		case FINISHED:
			render_finish_screen(player_score, opponent_score);
			break;
		case OPP_SERVICE:
		case STARTED:
		case EXIT:
			break;
		
	}
	sys_swap_buffers();
}


int process_events_task(SysEvent* event, int elapsedFrames, int period) {

	switch (event->type) {
		case CLOSE:
			change_state(EXIT);	
			break;
		case MOUSELBUTTONUP:
			if (gameState == STARTED || gameState == FINISHED) {
				change_state(LOADING_PLAYERS);
			} else if (gameState == PLAYER_SERVICE && ball_in_player_stick()) {
				play_player_pong_sound();
				change_state(PLAYER_RETURN);
			}
			break;
		case MOUSEMOTION:
			if (gameState == PLAYER_SERVICE || gameState == PLAYER_RETURN || gameState == OPP_RETURN) {
				mouse_move_player_stick(event->x, event->y);
				render(0);
			}
	} 
	return 0;
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
		render(time_delta);
		overlay_fadeout_alpha += overlay_fadeout;

	} else {
		init_game();
		play_start_sound();
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

bool equals(float a, float b) {
	return (bool)(fabs(a - b) <= ball.width);
}

int playing_task(int elapsedFrames, int pendingEvent) {

	float hit_wall_vector[3];
	static float to_position[2] = {0, 0 };
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

