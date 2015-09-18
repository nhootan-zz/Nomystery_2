#ifndef SEARCH_SPACE_H
#define SEARCH_SPACE_H

#include <vector>

class Operator;
class State;
class SearchNodeInfo;


class SearchNode {
    State * state_buffer;
    SearchNodeInfo &info;
public:
    SearchNode(State *state_buffer_, SearchNodeInfo &info_);

    State *get_state_buffer() {
      return state_buffer;
    }
    State get_state() const;

    bool is_open() const;
    bool is_closed() const;
    bool is_dead_end() const;

    int get_f() const;
    int get_g() const;
    int get_h() const;

    void open_initial(int h);
    void open(int h, const SearchNode &parent_node,
	      int op_cost);
    void reopen(const SearchNode &parent_node,
		int op_cost);
    void close();
    void mark_as_dead_end();

    void dump();
};


class SearchSpace {
    class HashTable;
    HashTable *nodes;
public:
    SearchSpace();
    ~SearchSpace();
    int size() const;
    SearchNode get_node(const State* state);
    void trace_path(const State &goal_state,
		    std::vector<const Operator *> &path) const;

    void dump();
	void clear();
    void statistics() const;
};

#endif
