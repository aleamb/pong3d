/**
	@file main.c
	@author Alejandro Ambroa (jandroz@gmail.com)
	@date 1 Oct 2017
	@brief Pong game in three dimensions. Inspired in pong game by Liquid Media (http://www.liquid.se/pong/). 
*/

#include "msys.h"
#include "sound.h"
#include "geometry.h"
#include "renderer.h"
#include "text.h"
#include "screens.h"
#include "tasks.h"
#include "pong3d.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifdef _WINDOWS
#include <windows.h>
#endif

void run_game();
void init_game();
int process_state(int, int, int, int, SysEvent* event);
void cleanup();
int process_events_task(SysEvent* event, int, int);

#ifdef _WINDOWS
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, INT nCmdShow) {
#else
int main(int argc, char** argv) {
#endif
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
	create_elements(WINDOW_WIDTH, WINDOW_HEIGHT, STAGE_BLOCKS);
	init_screens();
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
}

void run_game() {

	int elapsedFrames = 0;
	unsigned int startTime, elapsedTime;
	unsigned int period = (unsigned int)((1.0f / FPS) * 1000.0f) ;
	int pendingEvent = 0;
	SysEvent event;
	bool firstLoop = true;
	int reset_frames_counter = 0;
	int wait_time = 0;
	int time_last_frame = startTime = sys_get_ticks();
	GAME_STATE currentState = STARTING;
	change_state(STARTING);

	// Game loop.

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
			if (wait_time < 0) {
				wait_time = 0;
			} 
		}
		/**
			When ocurrs a event (like a mouse motion) waiting is interrupted, event processed
			and then current state is rendered. Next, the loop waits again with the updated time (prior time minus event processing time)
		*/
		pendingEvent = sys_wait(&event, wait_time);


	}

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

