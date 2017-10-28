#ifndef _CONFIG_H_
#define _CONFIG_H_

#define WINDOW_TITLE "Pong 3D"

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 378

#define STAGE_BLOCKS 8

/*
 segments for ball. The number of vertices would be BALL_SEGMENTS^2
 and angle for each segment 2*PI / BALL_SEGMENTS
 */
#define BALL_SEGMENTS 20
/*
  delay in frames to limit speed of computer stick
*/
#define OPPONENT_SAMPLE_ADJUST 30

// sample frecuency rate for sound synthetizer
#define SAMPLE_RATE 44100

// alpha value for overlay
#define OVERLAY_ALPHA 0.8f

#define INITIAL_BALL_SPEED_VECTOR { -0.1f,  0.05f, -1.3f }

#define STAGE_COLOR { 0.0f, 1.0f, 0.0f, 0.2f };

#define FPS 40

#define BALLS 12

#define FRAMES_DEC_FACTOR 1

#define INITIAL_BALL_VELOCITY_DECREMENT 10

#endif
