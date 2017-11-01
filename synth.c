/**
	@file synth.c
	@author Alejandro Ambroa
	@date 1 Oct 2017
	@brief Simple software synthetizer based on book "BasicSynth" by Daniel Mitchell. 
*/


#include "synth.h"
#include <math.h>
#include <stdlib.h>

#define P_2PI 6.283185307f

float oscillator(OSCILLATOR_TYPE type, float* ang, float incr) {
  double value = 0.0;
  switch(type) {
    case SIN:
      value = sin(*ang);
      *ang += incr;
      if (*ang >= P_2PI)
        *ang -= P_2PI;
      break;
    case TRIANGLE: {
      double triValue = *ang * M_PI_2;
      *ang += incr;
      if (triValue < 0.0)
        value = 1.0 + triValue;
      else
        value = 1.0 - triValue;
      if (*ang >= M_PI)
          *ang -= P_2PI;
      }
      break;
    case SAW:
      value = (*ang / M_PI) - 1.0;
      *ang += incr;
      if (*ang >= P_2PI)
        *ang -= P_2PI;
      break;
    case COS:
      value = cos(*ang);
      *ang += incr;
      if (*ang >= P_2PI)
        *ang -= P_2PI;
      break;
    case NONE:
      value = 0;
  }
  return (float)value;
}

int synthetize(SYNTH* synthParams, sample_t** out_samples, int sample_freq) {
  int state = 0;
  float value;
  float oldValue = 0.0f;
  float volume = synthParams->volume;

  int samples_count = (int)((float)sample_freq * synthParams->totalTime);
  sample_t *samples = (sample_t*)malloc(samples_count * sizeof(sample_t));

  int attackTimeSamples = (int)(synthParams->attackTime * (float)sample_freq);
  int decayTimeSamples = (int)(synthParams->decayTime * (float)sample_freq);
  int releaseTimeSamples = (int)(synthParams->releaseTime * (float)sample_freq);

  int envCount = attackTimeSamples;
  float slope = volume / (float)attackTimeSamples;

  float phaseIncrOsc1 = (P_2PI / (float)sample_freq) * synthParams->oscillator1_freq;
  float phaseIncrOsc2 = (P_2PI / (float)sample_freq) * synthParams->oscillator2_freq;
  float phase1 = 0.0f;
  float phase2 = 0.0f;

  for (int i = 0; i < samples_count; i++) {
    value = oscillator(synthParams->oscillator1_type, &phase1, phaseIncrOsc1);
    if (synthParams->oscillator2_type != NONE) {
      value = value * oscillator(synthParams->oscillator2_type, &phase2, phaseIncrOsc2);
    }
    // filter
    value = (synthParams->filterBeta1 * value) +  (synthParams->filterBeta2 * oldValue);
    oldValue = value;

    // amplitude envelope
    switch (state) {
      case 0:
      if (envCount > 0) {
        volume += slope;
        envCount--;
      } else {
        state = 1;
        envCount = decayTimeSamples;
        slope = (volume - synthParams->decayValue) / (float)decayTimeSamples;
      }
      case 1:
        if (envCount > 0) {
          envCount--;
          volume -= slope;
        } else {
          state = 2;
          envCount = releaseTimeSamples;
          slope = volume / (float)releaseTimeSamples;
        }
        break;
      case 2:
        if (envCount > 0) {
          envCount--;
          volume -= slope;
        } else {
          state = -1;
          volume = 0.0f;
        }
        break;
    }
    samples[i] = (sample_t)(volume * value);
  }

  // simple reverb
  float delayTime = synthParams->delayTime;
  float reverbSize = synthParams->reverbSize;
  int delaySamples = (int)(delayTime * (float)sample_freq);
  if (delaySamples > 0) {
    for (int i = 0; i < samples_count - delaySamples; i++) {
    samples[i + delaySamples] += samples[i] * (sample_t)reverbSize;
    }
  }
  *out_samples = samples;
  return samples_count;
}

void free_samples(sample_t *samples) {
	free(samples);
}


