/**
  @file tasks.h
  @author Alejandro Ambroa
  @date 1 Oct 2017
  @brief Tasks for all game logic. 
 */

#include "tasks.h"

#include "pong3d.h"
#include "renderer.h"
#include "screens.h"
#include "sound.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

float to_position[2] = { 0, 0 };
int framesToPosition = 0;
bool lookDesviation = false;

float overlay_fadeout;
int overlay_fadeout_frames;
float overlay_fadeout_alpha;
int player_score = 0;
int opponent_score = 0;
int balls = BALLS;

float ball_speed_vector[3];

bool resetFramesCounter = true;

int ball_frames_dec_factor = FRAMES_DEC_FACTOR;
int ball_decrement = INITIAL_BALL_VELOCITY_DECREMENT;

bool equals(float a, float b)
{
    return (bool)(fabs(a - b) <= ball.width);
}

int start_screen_task()
{
    sys_show_cursor(1);
    set_render();
    //change_state(STARTED);

    return 0;
}

int loading_players_task(int elapsedFrames)
{
    if (elapsedFrames == 0) {
        // overlay fading out velocity
        overlay_fadeout_frames = (int)(FPS * 0.3f);
        overlay_fadeout = OVERLAY_ALPHA / overlay_fadeout_frames;
        overlay_fadeout_alpha = overlay_fadeout;
    }
    if (elapsedFrames <= overlay_fadeout_frames) {
        renderer_clear_screen();
        render_stage();
        render_fadeout_overlay(overlay_fadeout_alpha);
        sys_swap_buffers();
        overlay_fadeout_alpha += overlay_fadeout;

    } else {
        play_start_sound();
        sys_show_cursor(0);
        player_score = 0;
        opponent_score = 0;
        balls = BALLS;
        change_state(PLAYER_SERVICE);
    }
    return 0;
}

static void set_initial_ball_velocity()
{
    srand((unsigned int)time(NULL));
    // 4 is a magic number
    ball_speed_vector[0] = ((rand() % 1) ? 1.0f : -1.0f) * stage.width / (FPS * (4 + rand() % 1));
    ball_speed_vector[1] = ((rand() % 1) ? 1.0f : -1.0f) * stage.height / (FPS * (4 + rand() % 1));
    ball_speed_vector[2] = -stage.large / (FPS - (ball_decrement * ball_frames_dec_factor));
}

int player_service_task(int elapsedFrames, int pendingEvent, SysEvent* event)
{
    if (elapsedFrames == 0) {
        ball_decrement = INITIAL_BALL_VELOCITY_DECREMENT;
        set_initial_ball_velocity();
        reset_ball_position();
        reset_player_stick_position();
        reset_opponent_stick_position();
        sys_mouse_center(WINDOW_WIDTH, WINDOW_HEIGHT);
        balls--;
        set_render();
        return 0;
    }
    if (pendingEvent) {
	if (event->type == MOUSELBUTTONUP && ball_in_player_stick()) {
        	change_state(PLAYER_RETURN);
        	play_player_pong_sound();
	}
        return 0;
    }
    return 0;
}

/**
  Main game logic.
 */

int playing_task(int elapsedFrames)
{

    float hit_wall_vector[3];
    int resetFrames = 0;

    if (ball_hit_wall(hit_wall_vector, &stage, &ball)) {
        ball_speed_vector[0] *= hit_wall_vector[0];
        ball_speed_vector[1] *= hit_wall_vector[1];
        play_wall_hit_sound();
    }
    // computer return ball
    if (gameState == OPP_RETURN) {
        if (elapsedFrames == 0) {
            // calculate vector from ball to center of screen for moving there
            to_position[0] = -opponent_stick.x;
            to_position[1] = -opponent_stick.y;
            // velocity of movement (magic number)
            framesToPosition = 10;
            if (framesToPosition > 0) {
                to_position[0] /= framesToPosition;
                to_position[1] /= framesToPosition;
            } else {
                to_position[0] = 0;
                to_position[1] = 0;
            }
        }
        // if ball is in player Z coord...
        if (equals(ball.z - ball.width, player_stick.z)) {
            if (ball_in_player_stick()) { // test if hits in player stick
                play_player_pong_sound();
                // invserse Z component of velocity
                ball_speed_vector[2] *= -1.0f;
                change_state(PLAYER_RETURN);
                // for float precision problems, we must move the ball to positin where stick surface touch ball.
                ball.z = player_stick.z - ball.width;
                ball.model_matrix[14] = ball.z;

                to_position[0] = to_position[1] = 0;

                // increase ball speed
                ball_decrement++;
                int sub_frames = ((FPS - ball_frames_dec_factor * ball_decrement));
                if (sub_frames > 0)
                    ball_speed_vector[2] = -stage.large / sub_frames;

                // register point where player hits the ball for, in next frames, calculate desviation vector to apply to ball
                player_stick.xprev = player_stick.x;
                player_stick.yprev = player_stick.y;
                player_stick.zprev = player_stick.z;
                // enable apply desviation vector
                lookDesviation = true;

            } else {
                change_state(OPP_WINS);
            }
        }
        // move stick to center
        if (!equals(opponent_stick.x, 0.0) || !equals(opponent_stick.y, 0.0))
            move_opponent_stick(opponent_stick.x + to_position[0], opponent_stick.y + to_position[1]);
        // no reset frame counter
        resetFrames = 0;
    } else if (gameState == PLAYER_RETURN) { // player returns ball

        if (equals(ball.z + ball.width, opponent_stick.z)) { // if ball is in Z coord of computer stick...

            if (ball_in_opponent_stick()) {
                play_opponent_pong_sound();
                ball_speed_vector[2] *= -1.0f;
                ball.z = opponent_stick.z + ball.width;
                ball.model_matrix[14] = ball.z;
                change_state(OPP_RETURN);
                resetFrames = 1;

            } else {
                change_state(PLAYER_WINS);
            }
        } else {
            // Computer IA. Look ball position each 4 frames and got to position with velocity
            if (elapsedFrames == 4) {
                to_position[0] = ball.x - opponent_stick.x;
                to_position[1] = ball.y - opponent_stick.y;
                framesToPosition = (int)fabs((opponent_stick.z - ball.z) / ball_speed_vector[2]);
                if (framesToPosition > 0) {
                    to_position[0] /= framesToPosition;
                    to_position[1] /= framesToPosition;
                } else {
                    to_position[0] = 0;
                    to_position[1] = 0;
                }
                // reset frames counter to check ball position again
                resetFrames = 1;
            }
            // move computer stick to calculated ball position
            if (!equals(opponent_stick.x, ball.x) || !equals(opponent_stick.y, 0.0))
                move_opponent_stick(opponent_stick.x + to_position[0], opponent_stick.y + to_position[1]);
        }
        if (lookDesviation) {
            // desviation of ball depending on player's stick movement. 6.0 is a magic number to smooth ball desviation
            ball_speed_vector[0] += (player_stick.x - player_stick.xprev) * (6.0f / FPS);
            ball_speed_vector[1] += (player_stick.y - player_stick.yprev) * (6.0f / FPS);
            lookDesviation = false;
        }
    }
    // ball movement
    move_ball(ball.x + ball_speed_vector[0], ball.y + ball_speed_vector[1], ball.z + ball_speed_vector[2]);
    set_render();
    return resetFrames;
}

int opponent_wins_task(int elapsedFrames)
{

    if (elapsedFrames == 0) {
        opponent_score++;
        play_opponent_wins_sound();
        set_render();
    } else if (elapsedFrames > 90) {
        if (balls)
            change_state(OPP_SERVICE);
        else
            change_state(FINISHED);
        return 1;
    }
    return 0;
}

int player_wins_task(int elapsedFrames)
{

    if (elapsedFrames == 0) {
        player_score++;
        play_player_wins_sound();
        set_render();
    } else if (elapsedFrames > 90) {
        if (balls)
            change_state(PLAYER_SERVICE);
        else
            change_state(FINISHED);
        return 1;
    }
    return 0;
}

int ball_hit_wall(float* outVector, PONG_ELEMENT* pStage, PONG_ELEMENT* pBall)
{
    outVector[0] = 1.0;
    outVector[1] = 1.0;
    if (pBall->x >= (pStage->width2 - pBall->width)) {
        outVector[0] = -1.0f;
        move_ball(pStage->width2 - pBall->width, pBall->y, pBall->z);
        return 1;
    }
    if (pBall->x < (-pStage->width2 + pBall->width)) {
        outVector[0] = -1.0f;
        move_ball(-pStage->width2 + pBall->width, pBall->y, pBall->z);
        return 1;
    }
    if (pBall->y > (pStage->height2 - pBall->width)) {
        outVector[1] = -1.0f;
        move_ball(pBall->x, pStage->height2 - pBall->width, pBall->z);

        return 1;
    }

    if (pBall->y < (-pStage->height2 + pBall->width)) {
        outVector[1] = -1.0f;
        move_ball(pBall->x, -pStage->height2 + pBall->width, pBall->z);

        return 1;
    }

    return 0;
}

int opponent_service_task()
{
    ball_decrement = INITIAL_BALL_VELOCITY_DECREMENT;
    set_initial_ball_velocity();
    ball_speed_vector[2] *= -1.0f;
    reset_ball_position();
    reset_player_stick_position();
    reset_opponent_stick_position();
    sys_mouse_center(WINDOW_WIDTH, WINDOW_HEIGHT);
    move_opponent_stick(0.0f, 0.0f);
    move_ball(0, 0, opponent_stick.z + ball.width);
    change_state(OPP_RETURN);
    balls--;
    return 0;
}

int finished_task(int elapsedFrames)
{
    if (elapsedFrames == 0) {
        sys_show_cursor(1);
        set_render();
    }
    return 0;
}
