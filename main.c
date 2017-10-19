#include "msys.h"
#include "sound.h"
#include "geometry.h"
#include "renderer.h"
#include "text.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 378
#define FPS 60
#define BALLS 12
#define SAMPLE_RATE 44100
#define TASK_SIZE 16

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
	EXIT_TASK
} TASK;

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

void render_main_screen();
void render_scores();

void mouse_move_player_stick(int mx, int my);

void change_state(GAME_STATE state);


TASK tasks[TASK_SIZE];
int task_index = -1;
TASK poll_task();
void send_task(TASK task, bool restart_frames_count);
bool restart_task_frames_count;

void cleanup();


float overlay_fadeout;
int  overlay_fadeout_frames;
float overlay_fadeout_alpha;


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
	send_task(RENDER_START_SCREEN_TASK, true);
	

	while (run) {
		startTime = sys_get_ticks();
		TASK task = poll_task();
		if (restart_task_frames_count)
			taskElapsedFrames = 0;
		switch(task) {
			case RENDER_START_SCREEN_TASK:
				renderer_clear_screen();
				render_start_screen_task(taskElapsedFrames, period);
				sys_swap_buffers();
				break;
			case RENDER_PREPARING_PLAYERS_TASK:
				renderer_clear_screen();
				loading_players_task(taskElapsedFrames, period);
				sys_swap_buffers();
				break;
			case RENDER_MAIN_SCREEN_TASK:
				renderer_clear_screen();
				render_main_screen();
				sys_swap_buffers();
				break;
			case EXIT_TASK:
				run = false;
				break;
			default:
				break;	
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

void send_task(TASK newTask, bool restart_frames_count) {
	if (task_index == TASK_SIZE) {
		task_index = -1;
	}
	task_index++;
	tasks[task_index] = newTask;
	restart_task_frames_count = restart_frames_count;
}

TASK poll_task() {
	if (task_index >= 0) {
		return tasks[task_index--];
	}
	return IDLE_TASK;
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
			send_task(EXIT_TASK, false);
			break;
		case MOUSELBUTTONUP:
			if (gameState == STARTED) {
				change_state(LOADING_PLAYERS);
				send_task(RENDER_PREPARING_PLAYERS_TASK, true);
			}
			break;
		case MOUSEMOTION:
			if (gameState == PLAYER_SERVICE) {
				mouse_move_player_stick(event->x, event->y);
				send_task(RENDER_MAIN_SCREEN_TASK, true);
			}
	} 
	return 0;
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
		send_task(RENDER_PREPARING_PLAYERS_TASK, false);

	} else {
		init_game();
		play_start_sound();
		change_state(PLAYER_SERVICE);
		render_main_screen();
		send_task(RENDER_MAIN_SCREEN_TASK, true);
	}
	return 0;
}



