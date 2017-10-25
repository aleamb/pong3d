#include "msys.h"
#include "sound.h"
#include "geometry.h"
#include "renderer.h"
#include "text.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 378
#define FPS 40
#define BALLS 12
#define SAMPLE_RATE 44100

typedef enum {
	IDLE,
	STARTING,
	STARTED,
	LOADING_PLAYERS,
	PLAYER_SERVICE,
	PLAYER_RETURN,
	OPP_RETURN,
	OPP_SERVICE,
	PLAYER_WINS,
	PLAYER_LOSE,
	FINISHED,
	EXIT
}GAME_STATE;

GAME_STATE gameState, prevGameState;

int player_score;
int opponent_score;
int balls;


void run_game();
void init_game();
void render(int, int);
bool process_state(int, int, int, int);

// tasks
int render_start_screen_task(int);
int loading_players_task(int, int);
int player_service_task(int, int);
int process_events_task(SysEvent* event, int, int);
int player_service_task(int, int	);
int player_service_task(int, int);
int playing_task(int, int);

void render_main_screen();
void render_scores();

void mouse_move_player_stick(int mx, int my);
int ball_in_player_stick();
int ball_in_opponent_stick();
int ball_in_stick(float ball_x, float ball_y, float ball_width, PONG_ELEMENT* stick);
int ball_hit_wall(float* outVector, PONG_ELEMENT* stage, PONG_ELEMENT* ball);

int relative_move_ball(float x, float y, float z);

void change_state(GAME_STATE state);

const float INITIAL_BALL_SPEED_VECTOR[] = { -0.002f,  0.002f, 0.07f };
const float OPP_STICK_RETURN_SPEED_VECTOR[] = { -0.1f,  0.1f, 0.0f };
const float OPP_STICK_SPEED_VECTOR[] = { -0.15f,  0.15f, 0.0f };

void cleanup();


float overlay_fadeout;
int  overlay_fadeout_frames;
float overlay_fadeout_alpha;


float ball_speed_vector[3];

bool resetFramesCounter = true;

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
	bool run = true;
	int period = ((1.0f / FPS) * 1000.0f) ;
	int pendingEvent = 0;
	SysEvent event;
	bool mustRender = false;
	GAME_STATE currentState = STARTING;
	change_state(STARTING);

	while (gameState != EXIT) {
		startTime = sys_get_ticks();
			if (currentState != gameState) {
			elapsedFrames = 0;	
			currentState = gameState;
		}
		printf("frames: %d\n", elapsedFrames);
		process_state(elapsedFrames, sys_get_ticks() - startTime, period, pendingEvent);

		elapsedTime = sys_get_ticks() - startTime;
		int wait_time = period - elapsedTime;
		printf("voy a esperar %d milis\n", wait_time);
		
		while(wait_time > 0 && (pendingEvent = sys_wait(&event, wait_time))) {
				printf("me ha interrumpido un evento\n");
				int t1 = sys_get_ticks();
				process_events_task(&event, elapsedFrames, period);
				wait_time -= (sys_get_ticks() - t1);
				printf("ahora voy a esperar %d milis\n", wait_time);
		}

		elapsedFrames++;
	}

}

void change_state(GAME_STATE state) {
	prevGameState = gameState;
	gameState = state;
}

bool process_state(int elapsedFrames, int time_delta, int period, int pendingEvent) {
	
	switch (gameState) {
		
		case STARTING:
			render_start_screen_task(elapsedFrames);
			break;
		case LOADING_PLAYERS:
			loading_players_task(elapsedFrames, period);
			break;
		case PLAYER_SERVICE:
			player_service_task(elapsedFrames, pendingEvent);
			break;
		case PLAYER_RETURN:
			playing_task(elapsedFrames, pendingEvent);
			break;
			
	}
	return false;
}

void render_main_screen() {
	render_stage();
	render_opponent_stick();
	render_ball();
	render_player_stick();
	render_balls_counter(balls);
	render_scores();
	render_shadows();
}

int process_events_task(SysEvent* event, int elapsedFrames, int period) {

	switch (event->type) {
		case CLOSE:
			change_state(EXIT);	
			break;
		case MOUSELBUTTONUP:
			if (gameState == STARTED) {
				change_state(LOADING_PLAYERS);
			} else if (gameState == PLAYER_SERVICE && ball_in_player_stick()) {
				change_state(PLAYER_RETURN);
			}
			break;
		case MOUSEMOTION:
			if (gameState == PLAYER_SERVICE || gameState == PLAYER_RETURN || gameState == OPP_RETURN) {
				mouse_move_player_stick(event->x, event->y);
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

void render_scores() {
	char text[16];
	sprintf(text, "YOU> %d", player_score);
	render_text(text, -stage.width  / 2.0f + 0.1, -stage.height / 2.0f + 0.05, 0.02, 48);
	sprintf(text, "Computer> %d", opponent_score);
	render_text(text, stage.width /2.0f - 0.15, -stage.height / 2.0f + 0.05, 0.02, 48);
}

int render_start_screen_task(int elapsedFrames) {
	renderer_clear_screen();
	render_stage();
	reset_overlay();
	render_overlay();
	render_text("Click on screen to begin", 0.0f, 0.0f, 0.02, 48);
	change_state(STARTED);
	sys_swap_buffers();
	return 0;
}

int loading_players_task(int elapsedFrames, int period) {
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
		overlay_fadeout_alpha += overlay_fadeout;
		sys_swap_buffers();
	} else {
		init_game();
		play_start_sound();
		change_state(PLAYER_SERVICE);
	}
	return 0;
}

int player_service_task(int elapsedFrames, int pendingEvent) {
	if (elapsedFrames == 0) {
		memcpy(ball_speed_vector, INITIAL_BALL_SPEED_VECTOR, sizeof(INITIAL_BALL_SPEED_VECTOR));
		renderer_clear_screen();
		render_main_screen();
		sys_swap_buffers();
	}
	//if (pendingEvent) {
		renderer_clear_screen();
		render_main_screen();
		sys_swap_buffers();
	//}
	return 0;
}

int playing_task(int elapsedFrames, int pendingEvent) {

	float hit_wall_vector[3];

	if (elapsedFrames > 6) {
		if ((ball.z + ball.width) >= player_stick.z && ball_in_player_stick()) {
			play_player_pong_sound();
			ball_speed_vector[2] *= -1.0f;
		}
		if (ball_hit_wall(hit_wall_vector, &stage, &ball)) {
			ball_speed_vector[0] *= hit_wall_vector[0];
			ball_speed_vector[1] *= hit_wall_vector[1];
			play_wall_hit_sound();
		}
		relative_move_ball(ball_speed_vector[0], ball_speed_vector[1], ball_speed_vector[2]);
		renderer_clear_screen();
		render_main_screen();
		sys_swap_buffers();
	} else if (pendingEvent) {
		renderer_clear_screen();
		render_main_screen();
		sys_swap_buffers();
	}

	return 0;
}

int relative_move_ball(float x, float y, float z) {
	ball.model_matrix[12] += x;
	ball.model_matrix[13] +=y;
	ball.model_matrix[14] += z; 
	ball.x += x;
	ball.y += y;
	ball.z += z;
	return 0;
}

int ball_hit_wall(float* outVector, PONG_ELEMENT* stage, PONG_ELEMENT* ball) {

	if ((ball->x > (stage->width / 2.0f - ball->width)) || (ball->x < (-stage->width / 2.0f + ball->width))) {
		outVector[0] = -1.0f;
		return 1;
	}
	if ((ball->y > (stage->height / 2.0f - ball->width)) || (ball->y < (-stage->height / 2.0f + ball->width))) {
		outVector[1] = -1.0f;
		return 1;
	}
	return 0;

}
