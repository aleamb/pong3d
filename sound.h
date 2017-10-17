#ifndef _SOUND_H_
#define _SOUND_H_

int setup_sound(int sample_freq);

void play_start_sound();
void play_player_pong_sound();
void play_opponent_pong_sound();
void play_player_wins_sound();
void play_opponent_wins_sound();
void dispose_sound();
void play_wall_hit_sound();

#endif
