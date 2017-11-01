/**
  @file screens.c
  @author Alejandro Ambroa
  @date 1 Oct 2017
  @brief Code for rendering screens. 
 */

#include "screens.h"
#include "geometry.h"
#include "renderer.h"
#include "text.h"
#include <stdio.h>
#include "msys.h"
#include "pong3d.h"
#include "tasks.h"

#define FONT_SIZE 48
#define TEXT_SIZE_SCALE 0.02f

float player_text_score_coords[2];
float computer_text_score_coords[2];
char score_text[16];

void init_screens() {
	player_text_score_coords[0] = -stage.width / 2.0f + 0.1f;
	player_text_score_coords[1] = -stage.height / 2.0f + 0.05f;
	computer_text_score_coords[0] = stage.width / 2.0f - 0.15f;
	computer_text_score_coords[1] = -stage.height / 2.0f + 0.05f;

}

void render_player_wins_screen() {
	render_stage();
	render_ball();
	render_opponent_stick();
	render_pong_element(&overlay);
	render_text("Player wins", 0.0f, 0.0f, TEXT_SIZE_SCALE, FONT_SIZE);

}

void render_opp_wins_screen() {
	render_stage();
	render_ball();
	render_opponent_stick();
	render_pong_element(&overlay);
	render_text("Computer wins", 0.0f, 0.0f, TEXT_SIZE_SCALE, FONT_SIZE);

}

void render_main_screen(int balls, int player_score, int computer_score) {
	render_stage();
	render_opponent_stick();
	render_ball();
	render_player_stick();
	render_scores(player_score, computer_score);
	render_shadows();
	render_balls_counter(balls);

}
void render_start_screen() {
	render_stage();
	reset_overlay();
	render_overlay();
	render_text("Click on screen to begin", 0.0f, 0.0f, TEXT_SIZE_SCALE, FONT_SIZE);

}

void render_finish_screen(int player_score, int computer_score) {
	render_start_screen();
	render_scores(player_score, computer_score);
}

void render_scores(int player_score, int computer_score) {
	sprintf(score_text, "YOU> %d", player_score);
	render_text(score_text, player_text_score_coords[0], player_text_score_coords[1], TEXT_SIZE_SCALE, FONT_SIZE);
	sprintf(score_text, "Computer> %d", computer_score);
	render_text(score_text, computer_text_score_coords[0], computer_text_score_coords[1], TEXT_SIZE_SCALE, FONT_SIZE); 
}

void render(int elapsedFrames) {
	renderer_clear_screen();
	switch (gameState) {
		case STARTING:
			render_start_screen();
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
		case LOADING_PLAYERS:
		case EXIT:
			break;

	}
	sys_swap_buffers();
}
