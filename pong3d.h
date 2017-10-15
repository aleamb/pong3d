#ifndef _CONFIG_H_
#define _CONFIG_H_

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 378
#define WINDOW_TITLE "Pong 3D"

#define FRAMES_PER_SECOND 60

// number of segments for stage
#define STAGE_BLOCKS 7

/*
 segments for ball. The number of vertices would be BALL_SEGMENTS^2
 and angle for each segment 2*PI / BALL_SEGMENTS
 */
#define BALL_SEGMENTS 20
// stage width
#define STAGE_BLOCK_WIDTH 1.0f
// large (deep) of each stage's block
#define STAGE_BLOCK_LARGE (STAGE_BLOCK_WIDTH / 5.0f)
// width for sticks
#define STICK_WIDTH (STAGE_BLOCK_WIDTH / 6.0f)

#define BALL_RADIUS (STAGE_BLOCK_WIDTH / 50.0f)

#define NUMBER_OF_BALLS 12

/*
  delay in frames to limit speed of computer stick
*/
#define OPPONENT_SAMPLE_ADJUST 30

// sample frecuency rate for sound synthetizer
#define SAMPLE_FREQ 44100

// alpha value for overlay
#define OVERLAY_ALPHA 0.8f

#define INITIAL_BALL_SPEED_VECTOR { -0.1f,  0.05f, -1.3f }

#define STAGE_COLOR { 0.0f, 1.0f, 0.0f, 0.2f };

#endif
