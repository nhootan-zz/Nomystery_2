#ifndef HEURISTIC_H
#define HEURISTIC_H
#include "state.h"
#include "problem.h"

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

