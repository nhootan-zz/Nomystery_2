#include "state.h"
#include "successor_generator.h"
#include <cstdlib>
#include <iostream>
#include <vector>
#include "globals.h"

using namespace std;

void SuccessorGenerator::generate_successors(const State &curr, vector<State> & succs, vector<int>& costs){

	vector<pair<int, int> > neighbors;
	vector<int> locations = curr.get_locations();
	int num_trucks = locations.size();
	for (int truck = 0; truck < num_trucks; truck++) {
		neighbors.clear();
		get_neighbor_costs(curr.get_problem()->graph, locations[truck], neighbors);
		for (int i = 0; i < neighbors.size(); ++i) {
			//cout << i << endl;
			//cout << neighbors[i].first << " " << neighbors[i].second << endl;
			State next = curr;
			int cost = 0;
			if(next.move(cost, truck, neighbors[i])){
				succs.push_back(next);
				costs.push_back(cost);
			}
		}
	}
	// cout << succs.size() << endl;
}

void SuccessorGenerator::get_neighbor_costs( vector<vector<int> >& graph ,int curr_loc, vector<pair<int, int> >& neighbors ){
	assert(curr_loc < graph.size());
	for ( int i = 0; i < graph[curr_loc].size(); ++i ) {
		int cost = graph[curr_loc][i];
		if( cost != NOT_CONNECTED ){
			neighbors.push_back( make_pair( i, cost ) );
		}
	}
}
