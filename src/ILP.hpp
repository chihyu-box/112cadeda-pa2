#pragma once

#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <glpk.h>
#include <cmath>

#include "Node.hpp"
#include "GraphInfo.hpp"
using namespace	std;

class ILP {
private:
	GraphInfo &graphInfo;
	int latency;
    double exact_latency;
	int AND_num;
	int OR_num;
	int NOT_num;
	int node_num;
	glp_prob *ilp;
	vector<int> ia;
	vector<int> ja;
	vector<double> ar;
    unordered_multimap<int, string> AND_scedule;
    unordered_multimap<int, string> OR_scedule;
    unordered_multimap<int, string> NOT_scedule;
public:
	ILP(GraphInfo &graphInfo, int latency, int AND_num, int OR_num, int NOT_num)
	 : graphInfo(graphInfo), latency(latency), exact_latency(0), AND_num(AND_num), OR_num(OR_num), NOT_num(NOT_num) {
	 	node_num = graphInfo.node_label.size();
	 	ia.push_back(0);
	 	ja.push_back(0);
	 	ar.push_back(0);
	}
	void solve() {
	    ilp = glp_create_prob();
	    glp_set_prob_name(ilp, "exact method");
	    glp_set_obj_dir(ilp, GLP_MIN);

	    define_variable();
	    set_start_time_constraints();
	    set_sequencing_relation_constraints();
	    set_resource_constraints();

	    int size = ia.size();

	    glp_load_matrix(ilp, size-1, ia.data(), ja.data(), ar.data());
		glp_write_lp(ilp, NULL, "model.lp");
	    glp_simplex(ilp, NULL);
	    glp_intopt(ilp, NULL);
		lpx_print_mip(ilp, "solution.txt");

	    exact_latency = ceil(glp_mip_obj_val(ilp)) - 1;
	    result();
	    print_out();
	    glp_delete_prob(ilp);
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
	void define_variable() {
		glp_add_cols(ilp, node_num*latency);
		int num = 1;
		for(int i=1; i<=node_num; ++i) {
			for(int j= 1; j<=latency; ++j) {
				string colName = graphInfo.node_label[i] + "_" + to_string(j);
				glp_set_col_name(ilp, num, colName.c_str());
				if(j<graphInfo.asap_level[graphInfo.node_label[i]] || j>graphInfo.alap_level[graphInfo.node_label[i]]) {
					glp_set_col_bnds(ilp, num, GLP_FX, 0.0, 0.0);
				}
				else {
			    	glp_set_col_bnds(ilp, num, GLP_LO, 0.0, 0.0);  // 1 >= x >= 0
				}
			    glp_set_col_kind(ilp, num, GLP_IV);            // x is integer
			    if(graphInfo.node_label[i] != "tail") {
			    	glp_set_obj_coef(ilp, num, 0);
			    }
			    else {
			    	if(j>=graphInfo.asap_level[graphInfo.node_label[i]] && j<=graphInfo.alap_level[graphInfo.node_label[i]]) {
						glp_set_obj_coef(ilp, num, j);
					}
			    }
			    num++;
			}
		}
	}
	void set_start_time_constraints() {
	    int current_row_start = glp_get_num_rows(ilp) + 1;  // Get the starting row
	    for (int i = 1; i <= node_num; i++) {
	        int current_row = current_row_start + i - 1;
	        glp_add_rows(ilp, 1);
	        for (int j = graphInfo.asap_level[graphInfo.node_label[i]]; j <= graphInfo.alap_level[graphInfo.node_label[i]]; j++) { 
	            ia.push_back(current_row);                    // i row for ith node   
	            ja.push_back((i-1)*latency + j);              // j col for jth latency
	            ar.push_back(1.0);                            // value = 1
	        }
	        glp_set_row_bnds(ilp, current_row, GLP_FX, 1.0, 1.0); // x1 + x2 + ... + xl = 1, for l latency
	    }
	}

	void set_sequencing_relation_constraints() {
	    for (int i = 1; i <= node_num; ++i) {

	        auto node_name = graphInfo.node_label[i];
	        int node_col = (i - 1) * latency;
	        auto leaves = graphInfo.graph[node_name]->successors;

	        for (auto &leaf : leaves) {

	        	auto leaf_name = leaf->get_id();
	            int leaf_num = graphInfo.reverse_label[leaf_name];
	            int leaf_col = (leaf_num - 1) * latency;
	            // For each latency of the current node, add constraint to ensure it's less than the start time of the leave
                glp_add_rows(ilp, 1); // Add a new constraint
                int current_row = glp_get_num_rows(ilp);
	            for (int j = graphInfo.asap_level[node_name]; j <= graphInfo.alap_level[node_name]; ++j) {
	                ia.push_back(current_row);              
	                ja.push_back(node_col + j);       
	                ar.push_back(j);  // coefficient for the current node's latency
	            }
	            for (int k = graphInfo.asap_level[leaf_name]; k <= graphInfo.alap_level[leaf_name]; ++k) {
	            	ia.push_back(current_row);              
	                ja.push_back(leaf_col + k);       
	                ar.push_back(-k);  // coefficient for the leaf node's latency
	            }
                glp_set_row_bnds(ilp, current_row, GLP_UP, 0.0, -1.0);  // The bound of this constraint
	        }
	    }
	}
	void set_resource_constraints() {
	    for (int l = 1; l < latency; ++l) {
	    	which_resource_constraints(l, logic_gate::AND);
	        which_resource_constraints(l, logic_gate::OR);
	        which_resource_constraints(l, logic_gate::NOT);
	    }
	}
	void which_resource_constraints(int current_latency, logic_gate type) {
	    int new_row = glp_add_rows(ilp, 1); // Add a new constraint
	    int current_row = glp_get_num_rows(ilp);
	    bool exist = false;

	    for (int i = 1; i <= node_num; ++i) {
	        int col_index = (i - 1) * latency + current_latency;
	        if (graphInfo.asap_level[graphInfo.node_label[i]] <= current_latency && graphInfo.alap_level[graphInfo.node_label[i]] >= current_latency) {
	        	if (graphInfo.node_map[graphInfo.node_label[i]]->get_type() == type) {
		        	ia.push_back(current_row);
		        	ja.push_back(col_index);
		            ar.push_back(1);
		            exist = true;
		        }
	        }
	    }
	    if(exist == false) {
	    	glp_del_rows(ilp, 1, &new_row);
	    	return;
	    }
	    switch (type) {
			case logic_gate::AND:
	            glp_set_row_bnds(ilp, current_row, GLP_UP, 0.0, AND_num);
	            break;
	        case logic_gate::OR:
	            glp_set_row_bnds(ilp, current_row, GLP_UP, 0.0, OR_num);
	            break;
	        case logic_gate::NOT:
	            glp_set_row_bnds(ilp, current_row, GLP_UP, 0.0, NOT_num);
	            break;
	        default:
	        	break;
	    }
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
				double col_val = glp_mip_col_val(ilp, index);
			    if(col_val == 1) {;
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
};