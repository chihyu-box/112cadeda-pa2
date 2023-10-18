#pragma once

#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <gurobi_c++.h>
#include <cmath>

#include "Node.hpp"
#include "GraphInfo.hpp"
using namespace	std;

class GUROBI {
private:
	GraphInfo &graphInfo;
	int latency;
    double exact_latency;
	int AND_num;
	int OR_num;
	int NOT_num;
	int node_num;
    unordered_multimap<int, string> AND_scedule;
    unordered_multimap<int, string> OR_scedule;
    unordered_multimap<int, string> NOT_scedule;
    GRBEnv env;
    GRBModel model;
public:
	GUROBI(GraphInfo &graphInfo, int latency, int AND_num, int OR_num, int NOT_num)
	 : graphInfo(graphInfo), latency(latency), exact_latency(0), AND_num(AND_num), OR_num(OR_num), NOT_num(NOT_num), env(), model(env) {
	 	node_num = graphInfo.node_label.size();
	}
	void solve() {
		cout << "===== Start to defining variables and setting start timeing constraints. =====" << endl;
	    define_variable_and_set_start_time_constraints();
		cout << "===== Start to setting sequency relation constraints. =====" << endl;
	    set_sequencing_relation_constraints();
		cout << "===== Start to setting recource constraints. =====" << endl;
	    set_resource_constraints();

	    model.write("gurobi.lp");
	    cout << "========== SOLVING ==========" << endl;
	    model.optimize();

	    exact_latency = model.get(GRB_DoubleAttr_ObjVal)-1;
	    result();
	    print_out();
	}
   void write_out() {
        fstream fo("exact.txt", ios::out);
        fo << "ILP-based Sceduling Result" << endl;
        for(int i=1; i<=exact_latency; ++i) {
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
        fo << "LATENCY: " << exact_latency << endl;
        fo << "END" << endl;
    }
    int get_latency() {
    	return exact_latency;
    }
private:
	void define_variable_and_set_start_time_constraints() {
		GRBLinExpr objFunc;
		GRBLinExpr startCons;
		GRBVar var;
		string nodeName;
		for(int i=1; i<=node_num; ++i) {
			for(int j=1; j<=latency; ++j) {
				nodeName = graphInfo.node_label[i];
				if(j>=graphInfo.asap_level[nodeName] && j<=graphInfo.alap_level[nodeName]) {
					var = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, nodeName + "_" + to_string(j));
					if(graphInfo.node_label[i] == "tail") {
				    	objFunc += j*var;
				    }
				    startCons += var;
				}
			}
			model.addConstr(startCons == 1);
			startCons = 0;
		}
		model.setObjective(objFunc, GRB_MINIMIZE);
		model.update();
	}

	void set_sequencing_relation_constraints() {
		string nodeName;
		string leafName;
		vector<shared_ptr<Node>> leaves;
		GRBLinExpr seqCons;
	    for (int i = 1; i <= node_num; ++i) {
	        nodeName = graphInfo.node_label[i];
	        leaves = graphInfo.graph[nodeName]->successors;
	        for (auto &leaf : leaves) {
				leafName = leaf->get_id();
                for(int j=1; j<=latency; ++j) {
					if (j>=graphInfo.asap_level[nodeName] && j<=graphInfo.alap_level[nodeName]) {
		                GRBVar var = model.getVarByName(nodeName + "_" + to_string(j));
		                seqCons += j*var;
		            }
		            if (j>=graphInfo.asap_level[leafName] && j<=graphInfo.alap_level[leafName]) {
		                GRBVar var = model.getVarByName(leafName + "_" + to_string(j));
		                seqCons -= j*var;
		            }
				}
		        model.addConstr(seqCons <= -1);
		        seqCons = 0;
	        }
	    }
	    model.update();
	}
	void set_resource_constraints() {
	    for (int l = 1; l < latency; ++l) {
	    	choose_resource(l, logic_gate::AND);
	        choose_resource(l, logic_gate::OR);
	        choose_resource(l, logic_gate::NOT);
	    }
	}
	void choose_resource(int current_latency, logic_gate type) {
		string nodeName;
	    bool exist = false;
	    GRBLinExpr resCons;
	    for (int i=1; i<=node_num; ++i) {	
	    	nodeName = graphInfo.node_label[i];
	        if (graphInfo.node_map[nodeName]->get_type() == type && graphInfo.asap_level[nodeName] <= current_latency && graphInfo.alap_level[nodeName] >= current_latency) {
	        	GRBVar var = model.getVarByName(nodeName + "_" + to_string(current_latency));
	        	resCons += var;
	            exist = true;
	        }
  		}
	    if(exist == false) {
	    	return;
	    }
	    switch (type) {
			case logic_gate::AND:
	            model.addConstr(resCons <= AND_num);
	            break;
	        case logic_gate::OR:
	            model.addConstr(resCons <= OR_num);
	            break;
	        case logic_gate::NOT:
	            model.addConstr(resCons <= NOT_num);
	            break;
	        default:
	        	break;
	    }
	    model.update();
	}
	void print_out() {
        cout << "ILP-based Sceduling Result" << endl;
        for(int i=1; i<=exact_latency; ++i) {
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
        cout << "LATENCY: " << exact_latency << endl;
        cout << "END" << endl;

	}
	void result() {
		int index = 1;
		for(int i = 1; i <= node_num; ++i) {
			for(int j = 1; j <= latency; ++j) {
				double varValue;
				if (j>=graphInfo.asap_level[graphInfo.node_label[i]] && j<=graphInfo.alap_level[graphInfo.node_label[i]]) {
					string nodeName = graphInfo.node_label[i] + "_" + to_string(j);
					GRBVar var = model.getVarByName(nodeName);
					varValue = var.get(GRB_DoubleAttr_X);
					if(varValue == 1) {;
				    	string name = graphInfo.node_label[i];
				    	logic_gate type = graphInfo.node_map[name]->get_type();
				    	if(type == logic_gate::AND) {
				    		AND_scedule.insert({j, name});
				    	}
				    	else if(type == logic_gate::OR) {
				    		OR_scedule.insert({j, name});
				    	}
				    	else if(type == logic_gate::NOT) {
				    		NOT_scedule.insert({j, name});
				    	}
				    } 
					index++;
				}
			}
		}
	}
};