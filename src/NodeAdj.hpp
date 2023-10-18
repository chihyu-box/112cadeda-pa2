#pragma once

#include <vector>
#include <memory>
#include "Node.hpp"

using namespace std;

struct NodeAdj {
	vector<shared_ptr<Node>> predecessors;
	vector<shared_ptr<Node>> successors;
	int and_num;
	int or_num;
	int not_num;
};