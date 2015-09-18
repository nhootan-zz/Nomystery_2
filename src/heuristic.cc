#include "heuristic.h"
#include "globals.h"


int Heuristic::evaluate(State& state){
	if(problem->num_trucks == 1)
		return basic(state);
	else if(problem->num_trucks == 2)
		//return basic(state);
		return MSF(state);
	return 0;
}

int Heuristic::basic(State& state){
	if((problem->num_trucks == 1 || state.get_fuel() == 0)  && state.get_delivered() != all_delivered)
		return min_action_cost;
	return 0;
}

int Heuristic::MSF(State& state){
	// In this function we identify all the packages that should be
	// delivered and pass the origins and destinations of them
	// to a function that computes two minimum 
	// spanning trees covering all the input locations.
	// We are not considering packages that are in both 
	// trucks. Yes this can happen becuase of the current
	// generator but it does not cause any problem in terms
	// of soundness and it can save some space by prunning some states. 
	// So here we build the spanning tree for the packages
	// that are not delivered and are in just one of the trucks.
	
	assert(problem->num_trucks == 2);
	set<int> locations;
	vector<int> result;
	vector<int> t_locations = state.get_locations();
	vector<int> in_trucks = state.get_in_trucks();
	int trucks_xor = 0;
	int trucks_and = in_trucks[0] & in_trucks[1];

	if(t_locations[0] != t_locations[1])
		trucks_xor = in_trucks[0] ^ in_trucks[1];
	else
		trucks_xor = trucks_and;

	int not_delivered = all_delivered & ~state.get_delivered();

	/*cout << "truck0: ";
	g_dump_bitstring(in_trucks[0]);
	cout << " l" << t_locations[0]; 

	cout << " truck1: ";
	g_dump_bitstring(in_trucks[1]);
	cout << " l" << t_locations[1]; 

	cout << " not_delivered: ";
	g_dump_bitstring(not_delivered);
	
	cout << " trucks_xor: ";
	g_dump_bitstring(trucks_xor);*/

	for (int i = 0; i < problem->num_packages; ++i) {
		for (int t = 0; t < in_trucks.size(); t++) {
			if ((g_masks[i] & (trucks_xor & in_trucks[t])) != 0) {
				locations.insert(t_locations[t]);
				locations.insert(problem->p_initials[i]);
			}
		}
		if ((g_masks[i] & not_delivered & (~trucks_and)) != 0) {
			// cout << i << endl;
			locations.insert(problem->p_initials[i]);
			locations.insert(problem->p_goals[i]);
		}
	}
	
	set<int>::iterator it = locations.begin();
	set<int>::iterator end = locations.end();
	result.reserve(locations.size());
	///cout << " locations: ";
	for(; it != end; it++ ){
		result.push_back(*it);
		//cout << *it << " ";
	}
	/*cout << endl;
	for (int p = 0; p < problem->num_packages; ++p) {
		cout << "p" << p << ": " << problem->p_goals[p] << " - " << problem->p_initials[p] << endl;
	}*/
	
	
	if(locations.size() < 2){
		//cout << " h: " << 0 << endl;
		return 0;
	}
	
	//assert(t_locations[0] != t_locations[1]);
	
	int h = problem->MSF(result); 
	h = max((h/2) - state.get_fuel(), 0);
	// cout << " h: " << h << endl;
	// h is at most two times of the optimal value
	// we also need to deduct the amount of the fuel
	// that we have. 
	return h;
}


Heuristic::Heuristic(Problem* problem_) : problem(problem_){
	min_action_cost = problem->min_cost_edge;
	all_delivered = 0;
	for (int i = 0; i < g_masks.size(); ++i) {
		all_delivered = all_delivered | g_masks[i];
	}
}

