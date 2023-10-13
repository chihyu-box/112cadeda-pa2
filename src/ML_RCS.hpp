#pragma once

#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <deque>
#include <memory>
#include <algorithm>
#include "Node.hpp"

using namespace std;

class ML_RCS {
private:
    GraphInfo &graphInfo;
    int AND_num;
    int OR_num;
    int NOT_num;
    bool isComplete;
    int latency;
    unordered_map<string, double> cost;
    deque<string> candidate_AND;
    deque<string> candidate_OR;
    deque<string> candidate_NOT;   
    unordered_multimap<int, string> AND_scedule;
    unordered_multimap<int, string> OR_scedule;
    unordered_multimap<int, string> NOT_scedule;
public:
    ML_RCS(GraphInfo &graphInfo, int AND_num, int OR_num, int NOT_num) 
     : graphInfo(graphInfo), AND_num(AND_num), OR_num(OR_num), NOT_num(NOT_num), isComplete(false), latency(0) {}

    void execute() {
        set_cost();
        // print_cost();
        join(graphInfo.graph["head"]->successors, "head");
        sort_all();
        latency = 1;
        int size = 0;
        while(isComplete == false) {
            vector<string> mem_AND;
            vector<string> mem_OR;
            vector<string> mem_NOT;
            size = candidate_AND.size();
            for (int i = 0; i < AND_num; ++i) {
                if(i < size) {
                    auto it_AND = candidate_AND.begin();
                    AND_scedule.insert({latency, *it_AND});
                    mem_AND.push_back(*it_AND);  // Store in memory
                    candidate_AND.pop_front();
                }
            }
            // print_out();
            // cout << "---" << endl;
            // print_candidate();
            // cout << endl << endl;
            size = candidate_OR.size();
            for (int i = 0; i < OR_num; ++i) {
                if(i < size) {
                    auto it_OR = candidate_OR.begin();
                    OR_scedule.insert({latency, *it_OR});
                    mem_OR.push_back(*it_OR);  // Store in memory
                    candidate_OR.pop_front();
                }
            }
            // print_out();
            // cout << "---" << endl;
            // print_candidate();
            // cout << endl << endl;
            size = candidate_NOT.size();
            for (int i = 0; i < NOT_num; ++i) {
                if(i < size) {
                    auto it_NOT = candidate_NOT.begin();
                    NOT_scedule.insert({latency, *it_NOT});
                    mem_NOT.push_back(*it_NOT);  // Store in memory
                    candidate_NOT.pop_front();
                }
            }
            // print_out();
            // cout << "---" << endl;
            // print_candidate();
            // cout << endl << endl;
            for(const string &item : mem_AND) {
                join(graphInfo.graph[item]->successors);
            }
            for(const string &item : mem_OR) {
                join(graphInfo.graph[item]->successors);
            }
            for(const string &item : mem_NOT) {
                join(graphInfo.graph[item]->successors);
            }
            sort_all();
            latency++;
        }
        latency--;
        print_out();
    }
   void write_out() {
        fstream fo("heuristic.txt", ios::out);
        fo << "Heuristic Scheduling Result" << endl;
        for(int i=1; i<=latency; ++i) {
            fo << i << ": " << "{" << flush;
            auto range = AND_scedule.equal_range(i);
            for(auto it = range.first; it != range.second; ++it) {
                fo << it->second << flush;
                if(next(it,1) != range.second) {
                    fo << " " << flush;
                }
            }
            fo << "} {" << flush;
            range = OR_scedule.equal_range(i);
            for(auto it = range.first; it != range.second; ++it) {
                fo << it->second << flush;
                if(next(it,1) != range.second) {
                    fo << " " << flush;
                }
            }
            fo << "} {" << flush;
            range = NOT_scedule.equal_range(i);
            for(auto it = range.first; it != range.second; ++it) {
                fo << it->second << flush;
                if(next(it,1) != range.second) {
                    fo << " " << flush;
                }
            }
            fo << "}" << endl;
        }
        fo << "LATENCY: " << latency << endl;
        fo << "END" << endl;
        fo.close();
    }
    int get_latency() {
        return latency;
    }
private:
    void set_cost() {
        for(auto &node : graphInfo.node_map) {
            string node_name = node.first;
            cost[node_name] = graphInfo.node_depth[node_name] + graphInfo.graph[node_name]->successors.size();
        }
    }
    void join(vector<shared_ptr<Node>>& root, const string &type = "normal") {
        for (const auto& leaf : root) {
            if (type == "normal") {
                leaf->decrease_fanin();
                if(leaf->fanin_ready()) {
                    switch(leaf->get_type())
                    {
                        case logic_gate::AND:
                            candidate_AND.push_back(leaf->get_id());
                            break;
                        case logic_gate::OR:
                            candidate_OR.push_back(leaf->get_id());
                            break;
                        case logic_gate::NOT:
                            candidate_NOT.push_back(leaf->get_id());    
                            break;
                        case logic_gate::TAIL:
                            isComplete = true;
                            break;
                        default:
                            break;
                    }
                }
            } 
            else {
                // cout << "in else:" << endl;
                // cout << leaf->get_id() << " " << leaf->get_fanin() << endl;
                leaf->decrease_fanin();
                join(graphInfo.graph[leaf->get_id()]->successors);
            }
        }
    }
    void sort_all() {
        sort_candidate(candidate_AND);
        sort_candidate(candidate_OR);
        sort_candidate(candidate_NOT);
    }
    void sort_candidate(std::deque<std::string>& candidate) {
        sort(candidate.begin(), candidate.end(), [this](const string& a, const string& b) {
            return this->cost.at(a) > this->cost.at(b);
        });
    }
    void print_out() {
        cout << "Heuristic Scheduling Result" << endl;
        for(int i=1; i<=latency; ++i) {
            cout << i << ": " << "{" << flush;
            auto range = AND_scedule.equal_range(i);
            for(auto it = range.first; it != range.second; ++it) {
                cout << it->second << flush;
                if(next(it,1) != range.second) {
                    cout << " " << flush;
                }
            }
            cout << "} {" << flush;
            range = OR_scedule.equal_range(i);
            for(auto it = range.first; it != range.second; ++it) {
                cout << it->second << flush;
                if(next(it,1) != range.second) {
                    cout << " " << flush;
                }
            }
            cout << "} {" << flush;
            range = NOT_scedule.equal_range(i);
            for(auto it = range.first; it != range.second; ++it) {
                cout << it->second << flush;
                if(next(it,1) != range.second) {
                    cout << " " << flush;
                }
            }
            cout << "}" << endl;
        }
        cout << "LATENCY: " << latency << endl;
        cout << "END" << endl;
    }
    void print_candidate() {
        cout << "AND:\n";
        for (const auto& candi : candidate_AND) {
            cout << candi << " ";
        }
        cout << endl;
        cout << "OR:\n";
        for (const auto& candi : candidate_OR) {
            cout << candi << " ";
        }
        cout << endl;
        cout << "NOT:\n";
        for (const auto& candi : candidate_NOT) {
            cout << candi << " ";
        }
        cout << endl;
    }
    void print_cost() {
        for(auto &v : cost) {
            cout << v.first << " " << v.second << endl;
        }
    }
};
