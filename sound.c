/**
	@file sound.c
	@author Alejandro Ambroa
	@date 1 Oct 2017
	@brief Generation of game sounds.
*/

#include "sound.h"
#include "msys.h"
#include "synth.h"
#include "SDL.h"

sample_t* player_pong_sound;
int player_pong_sound_samples;

sample_t* opponent_pong_sound;
int opponent_pong_sound_samples;

sample_t* player_score_sound;
int player_score_sound_samples;

sample_t* opp_score_sound;
int opp_score_sound_samples;

sample_t* wall_hit_sound;
int wall_hit_sound_samples;

sample_t* start_sound;
int start_sound_samples;

int init_sound(int sample_freq)
{

    if (sys_init_sound(sample_freq) < 0) {
        return -1;
    }
    SYNTH synthParams;

    synthParams.totalTime = 0.1f;
    synthParams.volume = 1.0f;
    synthParams.attackTime = 0.0f;
    synthParams.decayTime = 0.05f;
    synthParams.releaseTime = 0.05f;
    synthParams.decayValue = 0.3f;
    synthParams.filterBeta1 = 0.2f;
    synthParams.filterBeta2 = 0.3f;
    synthParams.oscillator1_type = TRIANGLE;
    synthParams.oscillator2_type = NONE;
    synthParams.oscillator1_freq = 700.0f;
    synthParams.delayTime = 0.5f;
    synthParams.reverbSize = 0.1f;
    player_pong_sound_samples = synthetize(&synthParams, &player_pong_sound, sample_freq);

    synthParams.oscillator1_freq = 350.0f;
    synthParams.oscillator2_freq = 350.0f;
    opponent_pong_sound_samples = synthetize(&synthParams, &opponent_pong_sound, sample_freq);

    synthParams.oscillator1_freq = 500.0f;
    synthParams.oscillator2_freq = 500.0f;
    start_sound_samples = synthetize(&synthParams, &start_sound, sample_freq);

    synthParams.totalTime = 0.05f;
    synthParams.volume = 0.7f;
    synthParams.attackTime = 0.01f;
    synthParams.decayTime = 0.03f;
    synthParams.releaseTime = 0.01f;
    synthParams.decayValue = 0.7f;
    synthParams.filterBeta1 = 0.6f;
    synthParams.filterBeta2 = 0.4f;
    synthParams.oscillator1_type = SIN;
    synthParams.oscillator2_type = SIN;
    synthParams.oscillator1_freq = 200.0f;
    synthParams.oscillator2_freq = 400.0f;
    synthParams.delayTime = 0.0f;
    synthParams.reverbSize = 0.0f;
    wall_hit_sound_samples = synthetize(&synthParams, &wall_hit_sound, sample_freq);

    synthParams.totalTime = 1.0f;
    synthParams.volume = 1.0f;
    synthParams.attackTime = 0.2f;
    synthParams.decayTime = 0.8f;
    synthParams.releaseTime = 0.4f;
    synthParams.decayValue = 0.1f;
    synthParams.filterBeta1 = 0.4f;
    synthParams.filterBeta2 = 0.4f;
    synthParams.oscillator1_type = SIN;
    synthParams.oscillator2_type = COS;
    synthParams.oscillator1_freq = 1046.50f;
    synthParams.oscillator2_freq = 2.0f;
    synthParams.delayTime = 0.0f;
    synthParams.reverbSize = 0.0f;
    player_score_sound_samples = synthetize(&synthParams, &player_score_sound, sample_freq);

    synthParams.totalTime = 0.6f;
    synthParams.volume = 1.0f;
    synthParams.attackTime = 0.01f;
    synthParams.decayTime = 0.3f;
    synthParams.releaseTime = 0.3f;
    synthParams.oscillator1_freq = 323.5f;
    synthParams.oscillator2_freq = 30.0f;
    opp_score_sound_samples = synthetize(&synthParams, &opp_score_sound, sample_freq);

    return 0;
}

void play_start_sound()
{
    sys_play_sound(start_sound, start_sound_samples * sizeof(sample_t));
}
void play_player_pong_sound()
{
    sys_play_sound(player_pong_sound, player_pong_sound_samples * sizeof(sample_t));
}
void play_opponent_pong_sound()
{
    sys_play_sound(opponent_pong_sound, opponent_pong_sound_samples * sizeof(sample_t));
}

void play_player_wins_sound()
{
    sys_play_sound(player_score_sound, player_score_sound_samples * sizeof(sample_t));
}

void play_opponent_wins_sound()
{
    sys_play_sound(opp_score_sound, opp_score_sound_samples * sizeof(sample_t));
}
void play_wall_hit_sound()
{
    sys_play_sound(wall_hit_sound, wall_hit_sound_samples * sizeof(sample_t));
}

void dispose_sound()
{
    free_samples(player_pong_sound);
    free_samples(opponent_pong_sound);
    free_samples(player_score_sound);
    free_samples(opp_score_sound);
    free_samples(wall_hit_sound);
}
