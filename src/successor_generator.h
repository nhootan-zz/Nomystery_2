#ifndef SUCCESSOR_GENERATOR_H
#define SUCCESSOR_GENERATOR_H
#include "state.h"
#include <iostream>
#include <vector>

class State;
using namespace std;


class SuccessorGenerator {
public:
    void generate_successors( const State &curr, vector<State> & succs, vector<int> & costs );
    void get_neighbor_costs( vector<vector<int> >& g_graph, int curr_loc, vector<pair<int, int> >& neighbors );
};

#endif
