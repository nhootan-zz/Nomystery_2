#ifndef STATE_H
#define STATE_H
#include "problem.h"
#include <iostream>
#include <vector>
using namespace std;

class StateInfo{

public:
	Problem* problem;
	int h;
	vector<int> p_locations;
	vector<int> t_locations;
	StateInfo(char*, Problem* problem_);
};


class State{

private:
	enum {UNKNOWN = -1};
	enum {SECOND = 0, FIRST = 1};
	Problem* problem;
	int delivered;
	int* in_truck;
	int* location;
	// since the current version can support upto 2 trucks we just need to keep track
	// of one fuel variable
	int initial_fuel;

	//determines if the input truck have the enough fuel to be moved to the input location
	bool can_move(int truck, pair<int, int>& location_cost);
	
	// The cost of moving the truck to the location. In case of two trucks the first truck can
	// be moved for free.
	int cost_move(int truck, pair<int, int>& location_cost);
	
public:

	State(StateInfo& info);
	State(Problem* problem_, int delivered_, vector<int>& in_trucks, vector<int>& locations, int fuel = UNKNOWN);
	~State();
	// State(Problem* problem_, vector<int>& truck, vector<int>& delivered, int loc);
	// State(const State& curr, int loc);
	// if it is possible moves the truck and returns the cost of moving
	// if the move is not possible it returns false
	bool move(int& cost, int truck, pair<int, int>& location_cost);
	bool operator==(const State &rhs);
	// State & operator=(const State &rhs);
	State(const State& other);
	void dump();
	size_t hash() const;
	vector<int> get_locations() const;
	int get_delivered() const {return delivered;} 
	vector<int> get_in_trucks() const;

	Problem* get_problem() const {return problem;} 
	int get_fuel(){return initial_fuel;}
};

#endif
