#include "problem.h"
#include "globals.h"
#include <iostream>
#include "open_list.h"

Parameters::Parameters( ){
	init( );
}

Parameters::Parameters( string problemfile ){
	init( );
	ifstream file;
	file.open(problemfile.c_str(), ios::in);
	char line[MAX_STR_SIZE];
	file.getline(line, MAX_STR_SIZE);
	string line_str = line;
	// boring string work to extract problem params from problem.pddl !!!
	assert(line_str.find("define") != -1);
	int lpos = line_str.find("-l") + 2;
	int tpos = line_str.find("-t");
	int ppos = line_str.find("-p") + 2;
	int dpos = line_str.find("---int");
	int ldpos = line_str.find("---e");
	int spos = line_str.find("-s") + 2;
	num_locations = atoi(line_str.substr(lpos, tpos - lpos).c_str());
	num_packages = atoi(line_str.substr(ppos, dpos - ppos).c_str());
	seed = atoi(line_str.substr(spos, ldpos - spos).c_str());
}

void Parameters::init( ){
	num_trucks = 2;
	num_locations = 6;
	num_packages = 6;
	
	/* n will stay fixed: I don't expect to go up to more than 10
	 * locations; of 45 possible edges, we'll have 15 = a 3rd then,
	 * which seems generous for a roadmap. at -l < 10, it'll be even
	 * more than a 3rd.
	 */
	n = 1.5;
	m = 25;
	c = 1;
	seed = 1;
	encoding = 0;
	num_paretos = 1;
	output_dir = ".";
}

void LocationInfo::dump( ){
	cout << "initials: ";
	g_dump_bitstring( initials );
	cout << "goals: ";
	g_dump_bitstring( goals );
}

Problem::Problem( Parameters* params_ ){
	params = params_;
	num_trucks = params->num_trucks;
	num_locations = params->num_locations;
	num_packages = params->num_packages;

	srandom(params->seed);

	// creates a random graph for the problem and assigns random
	// origins and destinations to the packages and the trucks
	create_random_graph();
	select_initials_goals();

	make_masks(num_packages);
	
	//Note that in the argument passing, 
	//we are exchanging goal with initial_state for backward search
	make_location_bitstrings(p_goals, p_initials);
	all_shortest_path();
}

// creating a new problem from an old one and possibly changing the origins and destinations of
// the trucks and the packages

Problem::Problem( const Problem& problem, 	
					   vector<int>& p_initials_,
					   vector<int>& p_goals_, 
					   vector<int>& t_initials_){
	
	params = problem.params;
	num_trucks = problem.num_trucks;
	num_locations = problem.num_locations;
	num_packages = problem.num_packages;
	p_initials = p_initials_;
	p_goals = p_goals_;
	t_initials = t_initials_;

	//Note that in the argument passing, 
	//we are exchanging goal with initial_state for backward search
	make_location_bitstrings(p_goals, p_initials);
}

void Problem::create_random_graph() {
	
	/* first, create a spanning tree:
	 * keep a set have (initially one random node x) and a set
	 * havenot (initially all nodes except x); iteratively draw
	 * one node x from have and one node y from havenot,
	 * connect x and y, and move y from havenot to have.
	 *
	 * will insert exactly num_locations-1 edges. well whatever...
	 * compute requested number of edges, count.
	 */
	int num_edges = 0;
	min_cost_edge = INT_MAX;
	graph.resize(num_locations);
	for (int i = 0; i < graph.size(); ++i) {
		graph[i].resize(num_locations, NOT_CONNECTED);
	}
	
	int total_num_edges = (int) (params->n * ((float) num_locations));
	
	vector<bool> have(num_locations, false);
	int rnd = random() % num_locations;
	//cout << rnd << endl;
	have[rnd] = true;
	int i, x, y = 0;
	while (true) {
		for (i = 0; i < num_locations; i++) {
			if (!have[i]) {
				break;
			}
		}
		if (i == num_locations) {
			break;
		}
		while (true) {
			x = random() % num_locations;
			//cout << x << endl;
			if (have[x]) {
				break;
			}
		}
		while (true) {
			y = random() % num_locations;
			//cout << y << endl;
			if (!have[y]) {
				break;
			}
		}
		have[y] = true;
		/* have to specify both directions, in this encoding.
		 */
		int c = (random() % params->m) + 1;
		//cout << c << endl;
		graph[x][y] = c;
		graph[y][x] = c;
		min_cost_edge = min(min_cost_edge, c);
		num_edges++;
	}
	/* now insert the requested nr of additional edges.
	 */
	int j = 0;
	while (num_edges < total_num_edges) {
		for (i = 0; i < num_locations - 1; i++) {
			for (j = i + 1; j < num_locations; j++) {
				if (graph[i][j] == NOT_CONNECTED) {
					break;
				}
			}
			if (j < num_locations) {
				break;
			}
		}
		if (i == num_locations - 1) {
			/* aready fully connected!
			 */
			break;
		}
		while (true) {
			x = random() % num_locations;
			y = random() % num_locations;
			// cout << x << endl;
			// cout << y << endl;
			
			if (x != y && graph[x][y] == NOT_CONNECTED) {
				break;
			}
		}
		int c = (random() % params->m) + 1;
		// cout << c << endl;
		graph[x][y] = c;
		graph[y][x] = c;
		min_cost_edge = min(min_cost_edge, c);
		num_edges++;
	}
}

// computes the cost of the minimum spanning forrest that includes two trees
// covering the input nodes

int Problem::MSF(vector<int>& input){
	/*cout << "Metric Graph:" << endl;
	for (int i = 0; i < C.size(); i++) {
		for (int j = 0; j < C[i].size(); j++) {
			cout << C[i][j] << " ";
		}
		cout << endl;
	}*/
	
	typedef pair<int, int> Edge;
	OpenList<Edge> queue;
	vector<vector<int> > clusters;
	vector<int> c_id;
	int result = 0;
	int num_edges = 0;
	c_id.resize(C.size());
	for (int i = 0; i < input.size(); i++) {
		vector<int> temp(1, input[i]);
		clusters.push_back(temp);
		c_id[input[i]] = clusters.size() - 1;
		for (int j = i + 1; j < input.size(); j++) {
			queue.insert(C[input[i]][input[j]], make_pair(input[i], input[j]));
		}
	}
	
	while(num_edges < (input.size() - 2)){
		/*cout << "clusters: ";
		for (int i = 0; i < input.size(); i++) {
			cout << c_id[input[i]] << " ";
		}
		cout << endl;*/
		// we need two trees. 
		assert(!queue.empty());
		int cost = queue.min();
		Edge edge = queue.remove_min();
		// cout << edge.first << " " << edge.second << " cost: " << cost << endl;
		if(c_id[edge.first] != c_id[edge.second]){
			// making sure that we are not introducing loops
			num_edges ++;
			result += cost;
			int old_id = max(c_id[edge.first], c_id[edge.second]);
			int new_id = min(c_id[edge.first], c_id[edge.second]);
			for (int i = 0; i < clusters[old_id].size(); i++) {
				c_id[clusters[old_id][i]] = new_id;
				clusters[new_id].push_back(clusters[old_id][i]);
			}
		}
	}

	/*cout << "clusters: ";
	for (int i = 0; i < input.size(); i++) {
		cout << c_id[input[i]] << " ";
	}
	cout << endl;*/
	
	set<int> processed;
	// cout << "clusters: " << endl;
	for (int i = 0; i < input.size(); i++) {
		if(processed.find(input[i]) == processed.end()){
			// cout << input[i];
			for (int j = i + 1; j < input.size(); j++) {
				if(c_id[input[i]] == c_id[input[j]]){
					// cout << " " << input[j];
					processed.insert(input[j]);
				}
			}
			// cout << endl;
		}
	}
	// cout << "result: " << result << endl;
	
	return result;
}

void Problem::dump_graph(){
	for (int i = 0; i < graph.size(); i++) {
		for (int j = 0; j < graph[i].size(); j++) {
			cout << graph[i][j] << " ";
		}
		cout << endl;
	}
}

void Problem::all_shortest_path(){
	dump_graph();
	int n = graph.size();
	C.resize(n);
	for (int i = 0; i < n; ++i) {
		int max_dis = (params->m) * n;
		C[i].resize(n, max_dis);
		for (int j = 0; j < n; ++j) {
			if(i == j)
				C[i][j] = 0;
			else if(graph[i][j] != NOT_CONNECTED)
				C[i][j] = graph[i][j];
		}
	}
	//print_2d_vector(C);
	for (int k = 0; k < n; ++k) {
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				C[i][j] = min(C[i][j], C[i][k] + C[k][j]);
			}
		}
	}
}

void Problem::select_initials_goals() {
	
	for (int i = 0; i < num_packages; i++) {
		p_initials.push_back(random() % num_locations);
		while (true) {
			int goal = random() % num_locations;
			if (p_initials.back() != goal) {
				p_goals.push_back(goal);
				break;
			}
		}
	}
	
	for (int i = 0; i < num_trucks; i++) {
		// int r = random();
		// cout << "r: " << r << endl;
		t_initials.push_back(random() % num_locations);
		// cout << "t: " << t_initials[0] << endl;
	}
	
}

void Problem::make_location_bitstrings( const vector<int>& initial_state, const vector<int>& goal) {
	int num_locations = graph.size();
	loc_info.clear();
	loc_info.resize(num_locations);
	
	for (int i = 0; i < initial_state.size(); ++i) {
		int loc = initial_state[i];
		/*if(i == 0)
		 cout << loc << endl;*/
		if (loc >= 0 && loc < num_locations)
			loc_info[loc].initials = loc_info[loc].initials | g_masks[i];
	}
	
	for (int i = 0; i < goal.size(); ++i) {
		int loc = goal[i];
		/*if(i == 0)
		 cout << loc << endl;*/
		if (loc >= 0 && loc < num_locations)
			loc_info[loc].goals = loc_info[loc].goals | g_masks[i];
	}
}

void Problem::output_pddl_file(FILE* output, vector<int>& initial_fuel) {
	
	int max_init_fuel = params->m;
	for (int i = 0; i < initial_fuel.size(); ++i) {
		max_init_fuel = max(max_init_fuel, initial_fuel[i]);
	}

	int printn = (int) (((float) 100) * params->n);
	int printc = (int) (((float) 100) * params->c);
	fprintf(output, "(define (problem transport-l%d-t%d-p%d---int100n%d-m%d---int100c%d---s%d---e%d)\n", num_locations,
		   num_trucks, num_packages, printn, params->m, printc, params->seed, params->encoding);
	fprintf(output, "(:domain transport-strips)\n\n");
	
	fprintf(output, "(:objects\n");
	int i = 0;
	for (i = 0; i < num_locations; i++) {
		fprintf(output, "l%d ", i);
	}
	
	fprintf(output, "- location\n");
	for (i = 0; i < num_trucks; i++) {
		fprintf(output, "t%d ", i);
	}
	fprintf(output, "- truck\n");
	for (i = 0; i < num_packages; i++) {
		fprintf(output, "p%d ", i);
	}
	fprintf(output, "- package\n");
	if(params->encoding == HARD_COST || params->encoding == HARD){
		for (i = 0; i <= max_init_fuel; i++) {
			fprintf(output, "level%d ", i);
		}
		fprintf(output, "- fuellevel\n");
	}
	fprintf(output, ")\n\n");
	
	fprintf(output, "(:init\n");
	for (int post = 0; post <= max_init_fuel; post++) {
		for (int pre = post; pre <= max_init_fuel; pre++) {
			if(params->encoding == HARD_COST || params->encoding == HARD)
				fprintf(output, "(sum level%d level%d level%d)\n", post, pre - post, pre);
		}
	}
	fprintf(output, "\n");
	
	for (i = 0; i < num_locations; i++) {
		for (int j = 0; j < num_locations; j++) {
			if (graph[i][j] != NOT_CONNECTED) {
				fprintf(output, "(connected l%d l%d)\n", i, j);
				if(params->encoding == HARD_COST || params->encoding == HARD)
					fprintf(output, "(fuelcost level%d l%d l%d)\n", graph[i][j], i, j);
				if(params->encoding == HARD_COST || params->encoding == SOFT || params->encoding == METRIC)
					fprintf(output, "(= (fuelcost l%d l%d) %d)\n", i, j, graph[i][j]);
			}
		}
	}
	fprintf(output, "\n");
	
	for (i = 0; i < num_trucks; i++) {
		fprintf(output, "(at t%d l%d)\n", i, t_initials[i]);
		if(params->encoding == HARD_COST || params->encoding == HARD)
			fprintf(output, "(fuel t%d level%d)\n", i, (initial_fuel[i]));
		else if(params->encoding == METRIC)
			fprintf(output, "(= (fuel t%d) %d)\n", i, initial_fuel[i]);
	}
	if(params->encoding == HARD_COST || params->encoding == SOFT)
		fprintf(output, "(= (total-cost) 0)\n");
	fprintf(output, "\n");
	
	for (i = 0; i < num_packages; i++) {
		fprintf(output, "(at p%d l%d)\n", i, p_initials[i]);
	}
	fprintf(output, ")\n\n");
	fprintf(output, "(:goal\n");
	fprintf(output, "(and\n");
	for (i = 0; i < num_packages; i++) {
		fprintf(output, "(at p%d l%d)\n", i, p_goals[i]);
	}
	fprintf(output, ")\n");
	fprintf(output, ")\n");
	if(params->encoding == HARD_COST || params->encoding == SOFT)
		fprintf(output, "(:metric minimize (total-cost))");
	fprintf(output, ")\n");
}

void Problem::dump() {
	cout << "truck location: " << t_initials[0] << endl;
	for (int var = 0; var < p_initials.size(); ++var) {
		cout << " p" << var << " " << p_initials[var] << " -- " << p_goals[var];
	}
	cout << endl;
	cout << "graph: " << endl;
	for (int i = 0; i < graph.size(); ++i) {
		for (int j = 0; j < graph[i].size(); ++j) {
			if (graph[i][j] == NOT_CONNECTED)
				cout << " -";
			else
				cout << " " << graph[i][j];
		}
		cout << endl;
	}
	cout << endl;
}


