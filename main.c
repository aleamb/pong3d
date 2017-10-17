#include "msys.h"
#include <stdbool.h>

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 378
#define FPS 40
#define BALLS 10
#define SAMPLE_RATE 44100

typedef enum {
  START,
  LOADING,
  PLAYER_SERVICE,
  PLAYER_RETURN,
  OPP_RETURN,
  OPP_SERVICE,
  PLAYER_WINS,
  PLAYER_LOSE,
  FINISHED,
  IDLE
}GAME_STATE;

GAME_STATE gameState;

int player_score;
int opponent_score;
int balls;

int (*current_task)(int, int, int);

//void run_game();
//void init_game();
//int task_start_screen(int, int, int);


int main(int argc, char** argv) {
	
	if (init_screen(WINDOW_WIDTH, WINDOW_HEIGHT) < 0) {
		quit(1);
	}
	if (init_sound(SAMPLE_RATE) < 0) {
		dispose_screen();
		quit(1);
	}
	return 0;
}
/*
void init_game() {
	balls = BALLS;
	gameState = START;
	executor_init(FPS);
	executor_set_task(task_start_screen);
}


void run_game() {
	int taskElapsedFrames = 0;
	int taskFrames;
	int startTime = currentTime = elapsedTime = 0;
	int elapsedTime = 0;
	bool running = true;
	int period = 1.0f / FPS;
	SDL_event event;

	while (running) {
		if (task()) {
			
			executeTask();
		}
		
	}
		startTime = SDL_GetTicks();
		switch (gameState) {	
			case INIT:
				task_start_screen(taskElapsedFrames, startTime, elapsedTime);
				break;
			case LOADING:
				task_loading_play(taskElapsedFrames
				break;
		}
		elapsedTime = SDL_GetTicks() - startTime;
		if (running && SDL_WaitEventTimeout(&event, period - elpasedTime )) {
			if (event.type == SDL_QUIT) {
				
			}
		}
		
		currentTime = SDL_GetTicks();
	}

}


int task_start_screen(int elapsedFrames, int startTime, int elapsedTime, int period) {

	render_stage();
  render_pong_element(&overlay);
  render_text(&startText, "Click on screen to begin", 0.0f, 0.0f, 0.02, 48);

}
*/

