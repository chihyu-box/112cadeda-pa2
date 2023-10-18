#pragma once

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <memory>
#include <stack>
#include <queue>
#include <functional>

#include "NodeAdj.hpp"
#include "Node.hpp"
#include "logic_gate.hpp"
using namespace std;

tuple<int, int, int> operator+(const tuple<int, int, int>& lhs, const tuple<int, int, int>& rhs) {
    return make_tuple(
        get<0>(lhs) + get<0>(rhs),
        get<1>(lhs) + get<1>(rhs),
        get<2>(lhs) + get<2>(rhs)
    );
}

struct GraphInfo {
	unordered_map<string, shared_ptr<NodeAdj>> graph;
	unordered_map<string, shared_ptr<Node>> node_map;
	unordered_map<int, string> node_label;
	unordered_map<string, int> reverse_label;
	unordered_map<string, int> node_depth;
	unordered_map<string, int> asap_level;
	unordered_map<string, int> alap_level;
	int node_count;

	std::unordered_set<std::shared_ptr<Node>> visited;

	GraphInfo() : node_count(0) {
		graph["head"] = make_shared<NodeAdj>();
		graph["tail"] = make_shared<NodeAdj>();
		node_map["head"] = make_shared<Node>("head", logic_gate::HEAD);
		node_map["tail"] = make_shared<Node>("tail", logic_gate::TAIL);
	}

	void insertNode(string name, logic_gate type, stack<string> container = {}) {

		auto node = make_shared<Node>(name, type);
		auto nodeAdj = make_shared<NodeAdj>();

		switch(type) {
		case logic_gate::INPUT:
			// HEAD POINTS TO INPUT
			graph["head"]->successors.push_back(node);
			// INPUT POINTS TO HEAD
			nodeAdj->predecessors.push_back(node_map["head"]);
			// INPUT FANIN +1
			node->increase_fanin();
			// HEAD FANOUT +1
			node_map["head"]->increase_fanout();
			break;
		case logic_gate::OUTPUT:
			// OUTPUT POINTS TO TAIL
			nodeAdj->successors.push_back(node_map["tail"]);
			// TAIL POINTS TO OUTPUT
			graph["tail"]->predecessors.push_back(node);
			// OUTPUT FANOUT +1
			node->increase_fanout();
			// TAIL FANIN +1
			node_map["tail"]->increase_fanin();
			break;
		case logic_gate::AND: case logic_gate::OR: case logic_gate::NOT: {
			// LABEL NODE
			node_label[++node_count] = name;
			reverse_label[name] = node_count;
			// GET OUTPUT NODE
			auto it = node_map.find(name);
			if(it != node_map.end()) {
			    node = it->second;
			    nodeAdj = graph[name];
			    node->set_type(type);
			}
			// LOOP FANIN NODE
			while(!container.empty()) {
	            string fanin_name = container.top();
	            container.pop();
	        // NODE POINTS TO FANIN_NAME
	            nodeAdj->predecessors.push_back(node_map[fanin_name]);
	        // FANIN_NAME POINTS TO NODE    
	            graph[fanin_name]->successors.push_back(node);
	        // NODE FANIN +1
	            node->increase_fanin();
	        // FANIN_NAME FANOUT +1
	            node_map[fanin_name]->increase_fanout();
	        }
	        break;
	    }
		default:
			break;
		}
		node_map[name] = node;	
		graph[name] = nodeAdj;
	}

	void computeNodeDepth() {
        dfsComputeDepth(node_map["tail"], 0);
    }

	void computeASAP() {
		for(auto &node_pair : node_map) {
			node_pair.second->save_fanin_count();
		}
	    BFS(node_map["head"], true, asap_level, -1);
		for(auto &node_pair : node_map) {
			node_pair.second->restore_fanin_count();
		}
    	removeLevels(asap_level);
	}

	void completeNodeLabel() {
		node_label[++node_count] = "tail";
		reverse_label["tail"] = node_count;
	}

	void computeALAP(int latency) {
    	BFS(node_map["tail"], false, alap_level, latency);
    	removeLevels(alap_level);	
	}

	void computeTypeNum() {
		visited.clear();
		countTypeNum(node_map["head"]);
	}

	void printTypeNum() {
		cout << "Type Num:" << endl;
	    for (const auto& pair : graph) {
	        cout << pair.first << " -> AND -> " << pair.second->and_num << endl;
	        cout << pair.first << " -> OR -> " << pair.second->or_num << endl;
	        cout << pair.first << " -> NOT -> " << pair.second->not_num << endl;
	    }
	    cout << endl;
	}

	void printGraph() {
	    cout << "Graph:" << endl;
	    for (const auto& pair : graph) {
	        cout << pair.first << " -> successor -> ";
	        for (const auto& successor : pair.second->successors) {
	            cout << successor->get_id() << " ";
	        }
	        cout << endl;
	        cout << pair.first << " -> predecessor -> ";
	        for (const auto& predecessor : pair.second->predecessors) {
	            cout << predecessor->get_id() << " ";
	        }
	        cout << endl;
	    }
	    cout << endl;
	}

	void printNodeMap() {
	    cout << "Node Map:" << endl;
	    for (const auto& pair : node_map) {
	        cout << pair.first << " -> " << pair.second->get_id() << endl;
	    }
	    cout << endl;
	}

	void printNodeLabel() {
	    cout << "Node Label:" << endl;
	    for (int i = 1; i <= int(node_label.size()); ++i) {
	        cout << i << " -> " << node_label.at(i) << endl;
	    }
	}

	void printASAPLevel() {
	    cout << "ASAP Level:" << endl;
	    vector<pair<string, int>> sorted_asap(asap_level.begin(), asap_level.end());
	    sort(sorted_asap.begin(), sorted_asap.end(), [](const auto& a, const auto& b) {
	        return a.second < b.second;
	    });

	    int current_level = 1;
    	cout << current_level << " ";
	    for (const auto& pair : sorted_asap) {
	        if (pair.second != current_level) {
	            cout << endl;
	            current_level = pair.second;
            	cout << current_level << " ";
	        }
	        cout << pair.first << " ";
	    }
	    cout << endl;
	}

	void printALAPLevel(int latency) {
	    cout << "ALAP Level:" << endl;
	    vector<pair<string, int>> sorted_alap(alap_level.begin(), alap_level.end());
	    sort(sorted_alap.begin(), sorted_alap.end(), [](const auto& a, const auto& b) {
	        return a.second > b.second;
	    });

	    int current_level = latency;
    	cout << current_level << " ";
	    for (const auto& pair : sorted_alap) {
	        if (pair.second != current_level) {
	            cout << endl;
	            current_level = pair.second;
	            cout << current_level << " ";
	        }
	        cout << pair.first << " ";
	    }
	    cout << endl;
	}


	void printNodeCount() {
	    cout << "Node Count: " << node_count << endl;
	}


private:
	void BFS(shared_ptr<Node> startNode, bool isASAP, unordered_map<string, int>& levelMap, int initialLatency) {
	    queue<shared_ptr<Node>> q;
	    q.push(startNode);
	    levelMap[startNode->get_id()] = initialLatency;

	    while (!q.empty()) {
	        auto currentNode = q.front();
	        q.pop();

	        vector<shared_ptr<Node>>& neighbors = isASAP ? graph[currentNode->get_id()]->successors : graph[currentNode->get_id()]->predecessors;

	        for (auto& neighbor : neighbors) {
	            if (isASAP) {
	                neighbor->decrease_fanin();
	                if (neighbor->fanin_ready() && levelMap.find(neighbor->get_id()) == levelMap.end()) {
	                    levelMap[neighbor->get_id()] = levelMap[currentNode->get_id()] + 1;
	                    q.push(neighbor);
	                }
	            } else {
	                neighbor->decrease_fanout();
	                if (neighbor->fanout_ready() && levelMap.find(neighbor->get_id()) == levelMap.end()) {
	                    levelMap[neighbor->get_id()] = levelMap[currentNode->get_id()] - 1;
	                    q.push(neighbor);
	                }
	            }
	        }
	    }
	}
	void removeLevels(unordered_map<string, int>& levelMap) {
	    for (auto it = levelMap.begin(); it != levelMap.end(); ) {
	        auto nodeType = node_map[it->first]->get_type();
	        if (nodeType == logic_gate::INPUT || nodeType == logic_gate::HEAD) {
	            it = levelMap.erase(it);
	        } else {
	            ++it;
	        }
	    }
	}
	void dfsComputeDepth(shared_ptr<Node> node, int depth) {
        if (node_depth.find(node->get_id()) != node_depth.end() && node_depth[node->get_id()] <= depth) {
            return;
        }

        node_depth[node->get_id()] = depth;

        for (auto& predecessor : graph[node->get_id()]->predecessors) {
            dfsComputeDepth(predecessor, depth + 1);
        }
    }

	tuple<int, int, int> countTypeNum(shared_ptr<Node>& node) {
        if (visited.find(node) != visited.end()) {
            return {0, 0, 0};
        }
        visited.insert(node);

        auto leaves = graph[node->get_id()]->successors;
        tuple<int, int, int> t;
        if (node->get_type() == logic_gate::TAIL) {
            return {0, 0, 0};
        }
        for (auto& leaf : leaves) {
            t = t + countTypeNum(leaf);
        }
        switch (node->get_type()) {
        case logic_gate::AND:
            get<0>(t) += 1;
            break;
        case logic_gate::OR:
            get<1>(t) += 1;
            break;
        case logic_gate::NOT:
            get<2>(t) += 1;
            break;
        default:
            break;
        }
        graph[node->get_id()]->and_num = get<0>(t);
        graph[node->get_id()]->or_num = get<1>(t);
        graph[node->get_id()]->not_num = get<2>(t);
        return t;
    }
};