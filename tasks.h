/**
	@file tasks.h
  	@author Alejandro Ambroa
  	@date 1 Oct 2017
  	@brief Tasks for all game logic. 
*/

#ifndef _TASKS_H_
#define _TASKS_H_

#include "msys.h"

int start_screen_task(int);
int loading_players_task(int, int, int);
int player_service_task(int, int, SysEvent*);
int opponent_service_task(int);
int playing_task(int, int);
int opponent_wins_task(int);
int player_wins_task(int);
int finished_task(int);

#endif
