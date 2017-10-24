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
#define FPS 60
#define BALLS 12
#define SAMPLE_RATE 44100
#define TASK_SIZE 12

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

typedef enum {
	IDLE_TASK,
	RENDER_START_SCREEN_TASK,
	RENDER_PREPARING_PLAYERS_TASK,
	RENDER_MAIN_SCREEN_TASK,
	PLAYER_SERVICE_TASK,
	PLAYING_TASK,
	EXIT_TASK
} TASK_TYPE;

typedef struct  {
	TASK_TYPE type;
	bool reset_frames_count;
	bool render;
}TASK;

TASK task;


int player_score;
int opponent_score;
int balls;


void run_game();
void init_game();

// tasks
int render_start_screen_task(int, int);
int loading_players_task(int, int);
int player_service_task(int, int);
int process_events_task(SysEvent* event, int, int);
int player_service_task(int, int	);
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
TASK tasks[TASK_SIZE];
TASK* poll_task();
void send_task(TASK_TYPE taskType, bool render, bool restart_frames_count);
int task_gap_index = -1;
int task_index = 0;

const float INITIAL_BALL_SPEED_VECTOR[] = { -0.002f,  0.002f, 0.07f };
const float OPP_STICK_RETURN_SPEED_VECTOR[] = { -0.1f,  0.1f, 0.0f };
const float OPP_STICK_SPEED_VECTOR[] = { -0.15f,  0.15f, 0.0f };

void cleanup();


float overlay_fadeout;
int  overlay_fadeout_frames;
float overlay_fadeout_alpha;


float ball_speed_vector[3];

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

	int taskElapsedFrames = 0;
	unsigned int startTime, elapsedTime;
	bool run = true;
	int period = ((1.0f / FPS) * 1000.0f) ;
	int pendingEvent = 0;
	SysEvent event;

	change_state(IDLE);
	change_state(STARTING);
	send_task(RENDER_START_SCREEN_TASK, true, true);


	while (run) {
		startTime = sys_get_ticks();
		TASK* task = poll_task();
		if (task) {
			if (task->reset_frames_count) {
				taskElapsedFrames = 0;
			}
			if (task->render) {
				renderer_clear_screen();
			}
			switch(task->type) {
				case RENDER_START_SCREEN_TASK:
					render_start_screen_task(taskElapsedFrames, period);
					break;
				case RENDER_PREPARING_PLAYERS_TASK:
					loading_players_task(taskElapsedFrames, period);
					break;
				case RENDER_MAIN_SCREEN_TASK:
					render_main_screen();
					break;
				case PLAYER_SERVICE_TASK:
					player_service_task(taskElapsedFrames, period);
					break;
				case PLAYING_TASK:
					playing_task(taskElapsedFrames, period);
					break;
				case EXIT_TASK:
					run = false;
					break;
				default:
					break;	
			}
			if (task->render) {
				sys_swap_buffers();
			}
		}


		if (pendingEvent) {
			process_events_task(&event, taskElapsedFrames, period);
		}
		elapsedTime = sys_get_ticks() - startTime;
		pendingEvent = sys_wait(&event, period - elapsedTime);

		taskElapsedFrames++;
	}

}

void change_state(GAME_STATE state) {
	prevGameState = gameState;
	gameState = state;
}

void send_task(TASK_TYPE taskType, bool render, bool restart_frames_count) {
	task_gap_index++;
	if (task_gap_index < TASK_SIZE) {
		tasks[task_gap_index].type = taskType;
		tasks[task_gap_index].render = render;
		tasks[task_gap_index].reset_frames_count = restart_frames_count;
	}
}

TASK* poll_task() {
	TASK* task;
	if (task_gap_index < 0) {
		return NULL;
	}
	task = &tasks[task_index];
	if (task_index >= task_gap_index) {
		task_index = 0;
	} else {
		task_index++;
	}
	return task;
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
			send_task(EXIT_TASK, false, false);
			break;
		case MOUSELBUTTONUP:
			if (gameState == STARTED) {
				change_state(LOADING_PLAYERS);
				send_task(RENDER_PREPARING_PLAYERS_TASK, true, true);
			} else if (gameState == PLAYER_SERVICE && ball_in_player_stick()) {
				send_task(PLAYER_SERVICE_TASK, false, true);
			}
			break;
		case MOUSEMOTION:
			if (gameState == PLAYER_SERVICE || gameState == PLAYER_RETURN || gameState == OPP_RETURN) {
				mouse_move_player_stick(event->x, event->y);
				send_task(RENDER_MAIN_SCREEN_TASK, true, true);
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

int render_start_screen_task(int elapsedFrames, int period) {
	render_stage();
	reset_overlay();
	render_overlay();
	render_text("Click on screen to begin", 0.0f, 0.0f, 0.02, 48);
	change_state(STARTED);
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
		render_stage();
		render_fadeout_overlay(overlay_fadeout_alpha);
		overlay_fadeout_alpha += overlay_fadeout;
		send_task(RENDER_PREPARING_PLAYERS_TASK, true, false);

	} else {
		init_game();
		play_start_sound();
		change_state(PLAYER_SERVICE);
		render_main_screen();
		send_task(RENDER_MAIN_SCREEN_TASK, true, true);
	}
	return 0;
}

int player_service_task(int elapsedFrames, int period) {
	memcpy(ball_speed_vector, INITIAL_BALL_SPEED_VECTOR, sizeof(INITIAL_BALL_SPEED_VECTOR));
	change_state(PLAYER_RETURN);
	send_task(PLAYING_TASK, true, true);
	return 0;
}

int playing_task(int elapsedFrames, int period) {

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
		send_task(PLAYING_TASK, true, true);
		render_main_screen();
	} else {
		send_task(PLAYING_TASK, false, false);
		render_main_screen();
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
