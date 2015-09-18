#include "state.h"
#include "globals.h"
#include <stdio.h>
#include <string.h>

StateInfo::StateInfo(char* str, Problem* problem_) {
	problem = problem_;
	// string template: (\([fuel|at|in], arg1, arg2\),)+
	//cout << str << endl;
	p_locations.resize(problem->num_packages);
	t_locations.resize(problem->num_trucks);

	char * pch = strtok(str," (),");
	vector<string> tokens;
	while (pch != NULL){
		tokens.push_back(string(pch));
		pch = strtok (NULL, " (),");
	}
	int i = 0;
	while(i < tokens.size()) {
		if(tokens[i] == "fuel"){
			// ignore fuel propositions
		}else if(tokens[i] == "at"){
			// arg1 is either a package or the truck, and arg2 is a location.
			string arg1 = tokens[i + 1];
			string arg2 = tokens[i + 2];
			int loc = get_id(arg2);

			if(arg1.find("t") == 0){
				int truck = get_id(arg1);
				t_locations[truck] = loc;

			}else{
				int package = get_id(arg1);
				if(problem->p_goals[package] == loc)
					// package is already delivered, we do not need to consider this package any more
					p_locations[package] = -1;
				else
					p_locations[package] = loc;
			}
		}else if(tokens[i] == "in"){
			// arg1 is a package, and arg2 is the truck.
			string arg1 = tokens[i + 1];
			string arg2 = tokens[i + 2];
			int package = get_id(arg1);
			int truck = get_id(arg2);
			// the value of the location becomes #locations + id of the truck
			p_locations[package] = problem->num_locations + truck;
		}
		// All propositions have 2 arguments
		i += 3;
	}
	for (int i = 0; i < p_locations.size(); ++i) {
		if(p_locations[i] >= problem->num_locations )
			p_locations[i] = t_locations[p_locations[i] - problem->num_locations];
	}
}


State::State(StateInfo& info){
	problem = info.problem;
	location = new int[info.t_locations.size()];
	in_truck = new int[info.t_locations.size()];
	initial_fuel = UNKNOWN;
	delivered = 0;
	
	for (int package = 0; package < info.p_locations.size(); ++package) {
		int loc = info.p_locations[package];
		if(loc == -1)
			// the package should be ignored.
			// (for now, the only possible reason is that it has already been delivered)
			continue;
		//if(loc < num_packages){
		/*if(g_p_goals[package] == loc){
		 // the package is delivered
		 delivered = delivered | g_masks[package];
		 }else*/
		delivered = delivered | g_masks[package];
		if (problem->p_initials[package] != loc){
			// cout << " p" << package << " -- l" << loc << endl;
			// the package is in a location which is not
			// either the initial or goal position. Therefore,
			// this state is not inside the abstraction and we should
			// ignore it for now.
			delivered = -1;
			// cout << "bilakh" << endl;
		}
		/*}else{
		 // the package is inside a truck
		 in_truck = in_truck | g_masks[package];
		 }*/
	}
	for (int i = 0; i < info.t_locations.size(); i++) {
		location[i] = info.t_locations[i];
		in_truck[i] = 0;
	}
}

State::State(Problem* problem_, int delivered_, vector<int>& in_trucks, vector<int>& locations, int fuel) : problem(problem_), delivered(delivered_) {
	location = new int[locations.size()];
	in_truck = new int[locations.size()];
	for (int loc = 0; loc < locations.size(); loc++) {
		assert(locations[loc] < problem->num_locations && locations[loc] >= 0);
		location[loc] = locations[loc];
		in_truck[loc] = in_trucks[loc];
	}
	initial_fuel = fuel;
}

State::~State(){
	delete[] location;
	delete[] in_truck;
}

//determines if the input truck have the enough fuel to be moved to the input location
bool State::can_move(int truck, pair<int, int>& location_cost){
	if(problem->num_trucks == 1)
		return true;
	if(problem->num_trucks == 2){
		if(truck == SECOND)
			return true;
		assert(truck == FIRST && initial_fuel != UNKNOWN);
		//cout << initial_fuel << endl;
		if(initial_fuel >= location_cost.second){
			//assert(false);
			return true;
		}
		return false;
	}
	cerr << "Error: The solver does not support more than two trucks" << endl;
	exit(1);
}

// The cost of moving the truck to the location. In case of two trucks the first truck can
// be moved for free.
int State::cost_move(int truck, pair<int, int>& location_cost){
	if(problem->num_trucks == 2 && truck == FIRST){
		// in case of two trucks the first truck can be moved for free.
		// cout << "Moving the truck with zero cost..." << endl;
		return 0;
	}
	return location_cost.second;
}

// if it is possible moves the truck and returns the cost of moving
// if the move is not possible it returns false
bool State::move(int& cost, int truck, pair<int, int>& location_cost){
	if(can_move(truck, location_cost)){
		//assert(truck != FIRST);
		cost = cost_move(truck, location_cost);
		int loc = location_cost.first;
		assert(loc < problem->num_locations && loc >= 0);
		location[truck] = loc;
		delivered = delivered | (problem->loc_info[loc].goals & in_truck[truck]);
		in_truck[truck] = (in_truck[truck] & ~problem->loc_info[loc].goals) | (problem->loc_info[loc].initials & ~delivered);
		//cout << " Move t" << truck << endl;
		//cout << " t" << FIRST << ": l" << location << endl;
		//cout << " t" << SECOND << ": l" << location << endl;

		//assert((in_truck[FIRST] & problem->loc_info[location[FIRST]].goals) == 0);
		//assert((in_truck[SECOND] & problem->loc_info[location[SECOND]].goals) == 0);
		if(problem->num_trucks == 2 && (location[FIRST] == location[SECOND])){
				// moving packages between trucks
				in_truck[FIRST] = in_truck[SECOND] = in_truck[FIRST] | in_truck[SECOND];
		}

		// assert((in_truck[FIRST] & problem->loc_info[loc].goals) == 0);
		// assert((in_truck[SECOND] & problem->loc_info[loc].goals) == 0);

		// making sure that if a package is delivered it is not in any of the trucks.
		for (int i = 0; i < problem->num_trucks; i++) {
			in_truck[i] = in_truck[i] & ~delivered;
		}
		
		
		if(problem->num_trucks == 2 && truck == FIRST){
			// update the initial fuel if we have one truck and the first is used.
			assert(initial_fuel != UNKNOWN);
			initial_fuel -= location_cost.second;
		}
		assert((in_truck[FIRST] & problem->loc_info[location[FIRST]].goals) == 0);
		assert((in_truck[SECOND] & problem->loc_info[location[SECOND]].goals) == 0);
		/*cout << "loc_str: ";
		g_dump_bitstring(problem->loc_info[loc].goals);
		cout << endl;*/
		return true;
	}
	return false;
}

vector<int> State::get_locations() const{
	vector<int> result;
	for (int i = 0; i < problem->num_trucks; i++) {
		result.push_back(location[i]);
	}
	return result;
}

vector<int> State::get_in_trucks() const{
	vector<int> result;
	for (int i = 0; i < problem->num_trucks; i++) {
		result.push_back(in_truck[i]);
	}
	return result;
}


void State::dump(){
	cout << "delivered:";
	g_dump_bitstring(delivered);
	for (int i = 0; i < problem->num_trucks; i++) {
		cout << " in_truck" << i << ":";
		g_dump_bitstring(in_truck[i]);
		cout << " location" << i << ": " << location[i];
	}
	cout << " fuel: " << initial_fuel;
}


size_t State::hash() const {
    // hash function adapted from Python's hash function for tuples.
    size_t hash_value = 0x345678;
    size_t mult = 1000003;
	for (int i = 0; i < problem->num_trucks; i++) {
		hash_value = (hash_value ^ in_truck[i]) * mult;
		mult += 82520 + 2 * i;
	}
	
	for (int i = 0; i < problem->num_trucks; i++) {
		hash_value = (hash_value ^ in_truck[i]) * mult;
		mult += 82520 + 2 * (problem->num_trucks) + 2 * i;
	}
	hash_value = (hash_value ^ delivered) * mult;
    hash_value += 97531;
    return hash_value;
}

/*State::State(Problem* problem_, vector<int>& truck, vector<int>& dlv, int loc): problem(problem_){
	location = loc;
	in_truck = 0;
	delivered = 0;
	for (int i = 0; i < truck.size(); ++i) {
		in_truck = in_truck | g_masks[truck[i]];
	}

	for (int i = 0; i < dlv.size(); ++i) {
		delivered = delivered | g_masks[dlv[i]];
	}
}*/

/*State::State(const State& curr, int loc){
	// move the truck to loc
	problem = curr.problem;
	location = loc;
	delivered = curr.delivered | (problem->loc_info[loc].goals & curr.in_truck);
	in_truck = (curr.in_truck & ~problem->loc_info[loc].goals) | (problem->loc_info[loc].initials & ~curr.delivered);
}*/

State::State(const State& other){
	problem = other.problem;
	delivered = other.delivered;
	in_truck = new int[problem->num_trucks];
	location = new int[problem->num_trucks];
	for (int i = 0; i < problem->num_trucks; i++) {
		in_truck[i] = other.in_truck[i];
		location[i] = other.location[i];
	}
	initial_fuel = other.initial_fuel;
}

bool State::operator==(const State& other){
	bool result = (problem == other.problem) && (delivered == other.delivered) && (initial_fuel == other.initial_fuel);
	for (int i = 0; i < problem->num_trucks; i++) {
		result = result && (in_truck[i] == other.in_truck[i]) && (location[i] == other.location[i]);
	}
	return result;
}
