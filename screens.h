#ifndef _SCREENS_H_
#define _SCREENS_H_

void render_main_screen(int balls, int player_score, int computer_score);
void render_scores(int player_score, int computer_score);
void render_player_wins_screen();
void render_opp_wins_screen(); 
void render_start_screen();
void render_finish_screen(int player_score, int computer_score);
void render(int);


#endif