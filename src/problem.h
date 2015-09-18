#ifndef PROBLEM_H
#define PROBLEM_H
#include <vector>
#include <string>
using namespace std;

class Parameters{
public:
	int num_packages;
	int num_locations;
	int num_edges;
	int num_trucks;
	float n; /* number of edges will be (int) (n * num_locations) */
	float c;
	int m; /* random edge weights will be drawn from {1..m} */
	int seed;
	int encoding;
	int num_paretos;
	string output_dir;

	Parameters( );
	Parameters( string problem_file );
	void init( );
};

class LocationInfo{
public:
	int initials;
	int goals;
	LocationInfo( ){
		initials = 0;
		goals = 0;
	}
	void dump( );
};

class Problem{
private:
	void create_random_graph( );
	void make_location_bitstrings( const vector<int>& initial_state, const vector<int>& goal );
	void select_initials_goals( );
	void all_shortest_path();
	void dump_graph();
	
public:
	Parameters* params;
	int num_packages;
	int num_locations;
	int num_edges;
	int num_trucks;
	int min_cost_edge;
	
	vector<int> p_initials;
	vector<int> p_goals;
	vector<int> t_initials;
	//vector<int> fuels;
	
	vector<vector<int> > graph;
	vector<vector<int> > C;
	vector<LocationInfo> loc_info;
	void output_pddl_file(FILE* output, vector<int>& initial_fuel );
	Problem( string problemfile );
	Problem( Parameters* params );
	Problem( const Problem& problem, vector<int>& p_initials, vector<int>& p_goals, vector<int>& t_initials );
	void dump( );
	int MSF(vector<int>& input);

	//void set_fuels(vector<int>& fuels_) {fuels = fuels_;}
};
#endif

