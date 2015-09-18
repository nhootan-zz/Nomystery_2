#ifndef HEURISTIC_H
#define HEURISTIC_H
#include "state.h"
#include "problem.h"
/*
 *  heuristic.h
 *  nomystery
 *
 *  Created by Hootan Nakhost on 06/10/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

class Heuristic{
	int all_delivered;
	int min_action_cost;
	Problem* problem;
public:
	Heuristic(Problem* problem);	
	int evaluate(State& state);
	int basic(State& state);
	int MSF(State& state);
};

#endif

