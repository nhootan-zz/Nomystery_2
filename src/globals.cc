#include "globals.h"
#include <fstream>
#include <iostream>

//returns the id of packages and locations
// e.g., l10 -> 10, p20 -> 20
int get_id(string str){
	// here, only the first element is a character
	string temp = str.substr(1, str.size());
	return atoi(temp.c_str());
}


void usage(Parameters* params) {
	printf("\nusage:\n");
	printf("\nOPTIONS   DESCRIPTIONS\n\n");
	printf("-l <int>    number of locations (>= 2; preset: %d)\n", params->num_locations);
	printf("-p <int>    number of packages (>= 1; preset: %d)\n\n", params->num_packages);
	printf("-t <int>    number of trucks (> 1; preset: %d)\n\n", params->num_trucks);
	printf("-n <float>  (int) (<-n> * <-l>) edges in location graph (>= 1; preset: %f)\n", params->n);
	printf("-m <int>    edge weights drawn from 1 .. <-m> (>= 1; preset: %d)\n\n", params->m);
	printf(
		   "-c <float>  constrainedness: (int) (<-c> * <optimalcost>) will be the initial fuel supply (>= 1; preset: %f)\n\n",
		   params->c);
	printf("-s <int>    random seed (>= 1; preset: %d)\n", params->seed);
	printf("-e < %d | %d | %d | %d > type of encoding (%d = hard; %d = hard_cost; %d = soft; %d = metric preset: %d)\n",
		   HARD, HARD_COST, SOFT, METRIC, HARD, HARD_COST, SOFT, METRIC, params->encoding);
	printf("-f <log_file> <problem.pddl> returns the actual goal-distance of evaluated states for the given log_file and the corresponding problem file  \n" );
	printf("-r <int> number of pareto-optimal points to generate (>=1; preset: %d)\n", params->num_paretos);
	printf("-o <string> the path to the output directory (preset: \"%s\")\n", params->output_dir.c_str());
}


bool process_command_line( int argc, char *argv[], Parameters* params ) {
	char option;
	while ( --argc && ++argv ) {
		if ( *argv[0] != '-' || strlen(*argv) != 2 ) {
			return false;
		}
		option = *++argv[0];
		switch (option) {
			default:
				if (--argc && ++argv) {
					switch (option) {
						case 'e':
							sscanf(*argv, "%d", &( params->encoding ) );
							break;
						case 'l':
							sscanf(*argv, "%d", &( params->num_locations ) );
							break;
						case 'f':
							char temp_str[100];
							sscanf(*argv, "%s", temp_str);
							g_log_file = string(temp_str);
							--argc;
							++argv;
							sscanf(*argv, "%s", temp_str);
							g_problem_file = string(temp_str);
							g_use_log = true;
							break;
						case 'p':
							sscanf( *argv, "%d", &( params->num_packages ) );
							break;
						case 'n':
							sscanf( *argv, "%f", & ( params->n ) );
							break;
						case 'm':
							sscanf( *argv, "%d", & ( params->m ) );
							break;
						case 'c':
							sscanf( *argv, "%f", & ( params->c ) );
							break;
						case 's':
							sscanf( *argv, "%d", &( params->seed ) );
							break;
						case 't':
							sscanf( *argv, "%d", &( params->num_trucks ) );
							break;
						case 'r':
							sscanf( *argv, "%d", &( params->num_paretos ) );
							break;
						case 'o':
							char temp_str1[100];
							sscanf(*argv, "%s", temp_str1);
							(params->output_dir) = string(temp_str1);
							//--argc;
							//++argv;
							break;
						default:
							cout << endl << endl << "unknown option: " << option << " entered" << endl << endl;
							return false;
					}
				} else {
					return false;
				}
			break;
		}
	}
	if ( params->num_locations < 2 || params->num_trucks < 2 || params->num_packages < 1 || params->n < 1 || params->m < 1 || params->seed < 1 ) {
		return false;
	}
	return true;
}

void g_dump_bitstring(int input) {
	if (input == 0) {
		cout << "--";
		return;
	}
	
	for (int i = 0; i < g_masks.size(); ++i) {
		if ((g_masks[i] & input) != 0) {
			cout << " p" << i;
		}
	}
}

void make_masks(int max_num){
	/* builds a vector v of bitstrings such that all the bits except i^th bit is 1
	 * and all the other bits are zeoros. e.g., v[5] = (100000)2
	 */
	g_masks.resize(max_num, 0);
	int m = 1;
	for (int i = 0; i < g_masks.size(); ++i) {
		g_masks[i] = m;
		m = m << 1;
	}
}

vector<StateInfo> read_log_file( string logfile, Problem* problem ){
	/* logfile template:
	 * number of states
	 * state strings
	 */
	
	ifstream file;
	file.open(logfile.c_str(), ios::in);
	char line[MAX_STR_SIZE];
	
	vector<StateInfo > state_list;
	while( !file.eof ( ) ){
		file.getline( line, MAX_STR_SIZE );
		StateInfo info( line, problem );
		file.getline( line, MAX_STR_SIZE );
		info.h = atoi( line );
		state_list.push_back( info );
	}
	return state_list;
}

bool g_use_log = false;
vector<int> g_masks;
string g_log_file = "";
string g_problem_file = "";

SuccessorGenerator g_successor_generator;

