#include "screens.h"
#include "geometry.h"
#include "renderer.h"
#include "text.h"
#include <stdio.h>

void render_player_wins_screen() {
	render_stage();
	render_ball();
	render_opponent_stick();
	render_pong_element(&overlay);
	render_text("Player wins", 0, 0, 0.02, 48);

}

void render_opp_wins_screen() {
	render_stage();
	render_ball();
	render_opponent_stick();
	render_pong_element(&overlay);
	render_text("Computer wins", 0, 0, 0.02, 48);

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
	render_text("Click on screen to begin", 0.0f, 0.0f, 0.02, 48);

}

void render_finish_screen(int player_score, int computer_score) {
	render_start_screen();
	render_scores(player_score, computer_score);
}

void render_scores(int player_score, int computer_score) {
	char text[16];
	sprintf(text, "YOU> %d", player_score);
	render_text(text, -stage.width  / 2.0f + 0.1, -stage.height / 2.0f + 0.05, 0.02, 48);
	sprintf(text, "Computer> %d", computer_score);
	render_text(text, stage.width /2.0f - 0.15, -stage.height / 2.0f + 0.05, 0.02, 48);
}


