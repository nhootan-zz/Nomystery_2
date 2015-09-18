#ifndef GLOBALS_H
#define GLOBALS_H

#include "successor_generator.h"
#include "solver.h"

#include <iostream>
#include <string>
#include <vector>
using namespace std;
#define MAX_STR_SIZE 500
#define NOT_CONNECTED -1
#define HARD 0
#define HARD_COST 1
#define SOFT 2
#define METRIC 3

class Problem;
class StateInfo;
class State;
class Solver;
/* returns the id of packages and locations
 * e.g., l10 -> 10, p20 -> 20
 */
int get_id(string str);

void usage(Parameters* params);
bool process_command_line(int argc, char *argv[], Parameters* params);

/* gets a bitstring as the input and ouputs the package names whoes bit are set to 1. */
void g_dump_bitstring(int input);

/* builds a vector v of bitstrings such that all the bits except i^th bit is 1
 * and all the other bits are zeoros. e.g., v[5] = (100000)2
 */
void make_masks(int max_num);
vector<StateInfo> read_log_file(string log_file, Problem* problem);
void solve(Solver& solver);

extern string g_log_file;
extern string g_problem_file;
extern bool g_use_log;
extern vector<int> g_masks;
extern vector<pair<State, int> > g_state_h;

extern SuccessorGenerator g_successor_generator;

#endif
