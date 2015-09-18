/*********************************************************************
 * Author: Hootan Nakhost
 *********************************************************************/
#include "solver.h"
#include "successor_generator.h"
#include "string.h"
#include "globals.h"
#include "problem.h"
#include "timer.h"
#include "heuristic.h"


#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <climits>
#include <algorithm>
#include <map>
#include <math.h>
#include <stdio.h>

#define FUEL 27

using namespace std;

Solver::Solver(){
	solution = UNKNOWN;
	all_delivered = 0;
	for (int i = 0; i < g_masks.size(); ++i) {
		all_delivered = all_delivered | g_masks[i];
	}
}

// gets an integer between 0 and #locations^#trucks and returns
// a possible configuration of initial locastions for the trucks
vector<int> Solver::map_id_locations(int id, int num_trucks, int num_locations){
	vector<int> result;
	result.push_back(id % num_locations);
	for (int i = 1; i < num_trucks; i++) {
		result.push_back((int)(id / (pow(num_locations, i))));
	}
	return result;
}

void Solver::run( Problem* problem_, int initial_fuel ) {
	problem = problem_;
	heuristic = new Heuristic(problem);
	vector<State> goals;
	vector<vector<int> > locations;
	int N = (int) pow(problem->num_locations, problem->num_trucks);
	for (int i = 0; i < N; i++) {
		vector<int> locations = map_id_locations(i, problem->num_trucks, problem->num_locations);
		vector<int> in_trucks;
		for (int i = 0; i < problem->num_trucks; i++) {
			in_trucks.push_back(problem->loc_info[locations[i]].initials);
		}
		goals.push_back(State(problem, 0, in_trucks, locations, initial_fuel));
	}
	solve(goals);
}
void Solver::reset( ) { 
	search_space.clear(); 
	open_list.clear(); 
}

void Solver::solve( vector<State>& states ) {
	reset( );
	for ( int i = 0; i < states.size(); ++i ) {
		SearchNode node = search_space.get_node( &states[i] );
		int h = heuristic->evaluate(states[i]);
		node.open_initial( h );
		open_list.insert( node.get_f(), h, node.get_state_buffer( ) );
	}
	while ( step( ) == IN_PROGRESS );
}

int Solver::step() {
	
    bool complete = false;
	SearchNode node = fetch_next_node(complete);
	// checking for the initial state
	if(is_initial_state(node)){
		solution = node.get_g();
		return SOLVED;
	}
	if(complete){
		return FAILED;
	}
	vector<State> successors;
	vector<int> costs;
	g_successor_generator.generate_successors(node.get_state(), successors, costs);
	for (int i = 0; i < successors.size(); i++) {
		//generated_states++;
		//successors[i].dump();
		SearchNode succ_node = search_space.get_node(&successors[i]);
		int cost = costs[i];

		if (succ_node.is_dead_end()) {
			// Previously encountered dead end. Don't re-evaluate.
			continue;
		} else if (succ_node.is_closed()) {
			// assert(succ_node.get_g() <= node.get_g() + cost);
			if(node.get_g() + cost < succ_node.get_g()) {
				succ_node.reopen(node, cost);
				open_list.insert(succ_node.get_f(), succ_node.get_h(),
								 succ_node.get_state_buffer());
			}
			// cout << endl;
			continue;
		} else if (succ_node.is_open()) {
			// Node is open, so we do not have to re-evaluate it.
			// However, we may have reached it on a cheaper path
			// than previously.
			
			// Node is open, so we do not have to re-evaluate it.
			// However, we may have reached it on a cheaper path
			// than previously.
			if(node.get_g() + cost < succ_node.get_g()) {
				succ_node.reopen(node, cost);
				open_list.insert(succ_node.get_f(), succ_node.get_h(),
								 succ_node.get_state_buffer());
			}

		} else {
			// We have not seen this state before.
			// Evaluate and create a new node.
			//evaluated_states++;

			int succ_h = heuristic->evaluate(successors[i]);
			succ_node.open(succ_h, node, cost);
			open_list.insert(succ_node.get_f(), succ_h,
							 succ_node.get_state_buffer());

			// cout << " cost: " << succ_h << endl;
		}
	}
	return IN_PROGRESS;
}

bool Solver::is_initial_state(SearchNode& node){
	State state = node.get_state();
	if(state.get_delivered() == all_delivered){
		vector<int> locations = state.get_locations();
		for (int i = 0; i < locations.size(); i++) {
			if(problem->t_initials[i] != locations[i]){
				return false;
			}
		}
		return true;
	}
	return false;
}
SearchNode Solver::fetch_next_node(bool& complete) {
	while (true) {
		if (open_list.empty()) {
            cerr << "Completely explored state space -- no solution!" << endl;
            exit(1);
            // TODO: Deal with this properly. step() should return
            //       failure.
        }

		State* state = open_list.remove_min();
		SearchNode node = search_space.get_node(state);
		
		// If the node is closed, we do not reopen it, as our heuristic
		// is consistent.
		if (!node.is_closed()) {
			node.close();
			assert(!node.is_dead_end());
			complete = false;
			return node;
		}
	}
}

void Solver::dump(){
	search_space.dump();
}

int Solver::get_cost(State& state){
	SearchNode node = search_space.get_node(&state);
	return node.get_h();
}

vector<vector<int> > Solver::get_pareto_optimal_solutions( Problem& problem ){
	Timer timer;
	vector<vector<int> > results;
	int initial_fuel = UNKNOWN;
	if(problem.num_trucks == 1){
		run( &problem, initial_fuel );
		vector<int> temp(1, solution); 
		results.push_back(temp);
		return results;
	}else{
		assert(problem.num_trucks == 2);
		initial_fuel = 0;
		int last_solution = solution;
		while(solution != 0 ){
			Timer local_timer;
			run( &problem, initial_fuel );
			local_timer.stop();
			if(solution != last_solution){
				vector<int> temp;
				temp.push_back(solution);
				temp.push_back(initial_fuel);
				cout << solution << " " << initial_fuel << " time: " << local_timer << endl;
				search_space.statistics();	
				results.push_back( temp );
			}
			initial_fuel ++;
			last_solution = solution;
		}
	}
	cout << "Total time: " << timer << endl;
	return results;
}

int main( int argc, char * argv[] ) {
	
	Parameters * params = new Parameters;
	if ( !process_command_line( argc, argv, params ) ) {
		usage( params );
		exit( 1 );
	}
	if( !g_use_log ) {
		
		Problem problem( params );
		
		Solver engine;
		cout << "Pareto optimal points: " << endl;
		vector<vector<int> > pareto_points = engine.get_pareto_optimal_solutions( problem );
		for (int i = 0; i < pareto_points.size(); i++) {
			for (int j = 0; j < pareto_points[i].size(); j++) {
				cout << pareto_points[i][j] << " ";
			}
			cout << endl;
		}

		if (params->num_paretos < 1){
			cerr << "Error the number of pareto solutions can not be less than one." << endl;
			exit(1);
		}
		vector<int> results;
		if(problem.num_trucks > 1){
			int num_paretos = pareto_points.size();
			cout << "total num paretos: " << num_paretos << endl;

			if(num_paretos % 2 == 0)
				num_paretos = (num_paretos / 2) + 1;
			else
				num_paretos = (num_paretos / 2);

			if(params->num_paretos > num_paretos){
				cerr << "Error: the number of meaningful pareto-optimal points to test: " << num_paretos
					 << " is less than what is requested: " << params->num_paretos << endl;
				exit(1);
			}
			//(params->num_paretos) --;
			int budget = num_paretos - params->num_paretos;
			int mod = budget % (params->num_paretos);
			budget = (budget / (params->num_paretos));
			cout << "mod: " << mod << endl;
			int index = 0;
			cout << "num interesting paretos: " << num_paretos << endl;
			while(results.size() != params->num_paretos){
				cout << "index: " << index << endl;
				assert(index <= num_paretos);
				results.push_back(index);
				if(mod > 0){
					index += (budget + 2);
					mod --;
				} else
					index += (budget + 1);
			}
		}
		for (int i = 0; i < results.size(); ++i) {
			cout << results[i] << " ";
		}
		cout << endl;
		for (int i = 0; i < results.size(); ++i) {
			vector<int> fuel;
			int index = results[i];
			for (int j = 0; j < pareto_points[index].size(); ++j) {
				int value = pareto_points[index][j] * params->c;
				fuel.push_back(value);
			}

			FILE * pFile;
			stringstream ss;
			ss << (i+1) << ".pddl";

			string problem_str = ss.str();
			if((i + 1) < 10)
				problem_str = string("0") + problem_str;
			string filename = params->output_dir + "/" + problem_str;
			pFile = fopen (filename.c_str() , "w");
			if (pFile == NULL){
				cerr << "Error: can not open the output file. File path: " << filename << endl;
				exit(1);
			}
			problem.output_pddl_file(pFile, fuel);
			fclose (pFile);
		}
	}else{
		// TODO: fix this part of reading the files 
		// read parameters from the problem_file
		// Parameters params_from_file( g_problem_file );
		// Problem problem( &params_from_file );
		// vector<StateInfo> state_list = read_log_file( g_log_file, &problem );
		// compute_costs( state_list, &problem );
	}
	return 0;
}

