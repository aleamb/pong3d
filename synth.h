#ifndef _SYNTH_H_
#define _SYNTH_H_


typedef enum {
    NONE,
    SIN,
    SAW,
    TRIANGLE,
    COS
}OSCILLATOR_TYPE;

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

#endif
