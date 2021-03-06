/**
	@file synth.c
	@author Alejandro Ambroa
	@date 1 Oct 2017
	@brief Simple software synthetizer based on book "BasicSynth" by Daniel Mitchell. 
*/

#ifndef _SYNTH_H_
#define _SYNTH_H_

typedef float sample_t;

typedef enum {
    NONE,
    SIN,
    SAW,
    TRIANGLE,
    COS
} OSCILLATOR_TYPE;

typedef struct {
    float totalTime;
    float volume;
    float attackTime;
    float decayTime;
    float releaseTime;
    float decayValue;
    float filterBeta1;
    float filterBeta2;
    OSCILLATOR_TYPE oscillator1_type;
    OSCILLATOR_TYPE oscillator2_type;
    float oscillator1_freq;
    float oscillator2_freq;
    float delayTime;
    float reverbSize;
} SYNTH;

int synthetize(SYNTH* synth_params, sample_t** samples_buffer, int sample_freq);
void free_samples(sample_t* samples);

#endif
