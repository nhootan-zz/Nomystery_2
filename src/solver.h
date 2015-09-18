#ifndef SOLVER_H
#define SOLVER_H

#include "fh_open_list.h"
#include "state.h"
#include "search_space.h"
#include "string.h"
#include "heuristic.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <climits>
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <algorithm>

using namespace std;

class Solver{
private:
	enum{UNKNOWN = -1};
	int solution;
	int all_delivered;
	Problem* problem;
	SearchSpace search_space;
	FhOpenList<State *> open_list;
	Heuristic* heuristic;
    enum { FAILED, SOLVED, IN_PROGRESS };
	// int get_optimal_solution( Problem& problem );
	int step( );
    void reset( );
	void solve( vector<State>& states );
	bool is_initial_state(SearchNode& node);
	SearchNode fetch_next_node( bool& complete );
	vector<int> map_id_locations(int id, int num_trucks, int num_locations);

public:
	Solver();
	void run( Problem* problem, int initial_fuel );
	int get_cost( State& state );
	void dump( );
	vector<vector<int> > get_pareto_optimal_solutions( Problem& problem );
};

#endif
