#ifndef _TASKS_H_
#define _TASKS_H_

#include "msys.h"

int start_screen_task(int);
int loading_players_task(int, int, int);
int process_events_task(SysEvent* event, int, int);
int player_service_task(int, int, SysEvent*);
int opponent_service_task(int);
int playing_task(int, int);
int opponent_wins_task(int);
int player_wins_task(int);
int finished_task(int);


extern int player_score;
extern int opponent_score;
extern int balls;


#endif
