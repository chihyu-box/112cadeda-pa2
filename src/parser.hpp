#pragma once

#include <fstream>
#include <sstream>
#include "GraphInfo.hpp"

using namespace std;

GraphInfo parser(fstream &fi) {

	GraphInfo graphInfo;

	if (!fi.is_open()) {
        cerr << "Failed to open the file." << endl;
        exit(1);
    }

    string buffer;
    while(fi >> buffer) {
        if(buffer == ".inputs")
            break;
    }
    // ENTER CASE: inputs
    while(fi >> buffer) {
        if(buffer == "\\") {
            continue;
        } else if(buffer[0] == '.') {
            break;
        } else {
            graphInfo.insertNode(buffer, logic_gate::INPUT);
        }
    }
    while(buffer != ".outputs") {
        fi >> buffer;
    }
    // ENTER CASE: outputs
    while(fi >> buffer) {
        if(buffer == "\\") {
            continue;
        } else if(buffer[0] == '.') {
            break;
        } else {
            graphInfo.insertNode(buffer, logic_gate::OUTPUT);
        }
    }
    while(buffer != ".names") {
        fi >> buffer;
    }
    // ENTER CASE: .names
    bool isEnd = false;
    while(isEnd == false) {
    	string line;
    	stack<string> container;
        getline(fi, line);
        stringstream ss(line);
        while(ss >> buffer) {
            container.push(buffer);
        }

        string gate_name = container.top();
        container.pop();

        fi >> buffer;

        logic_gate gate_type;
        if(buffer == "0") {
            gate_type = logic_gate::NOT;
        } else if(buffer.find('-') != std::string::npos) {
            gate_type = logic_gate::OR;
        } else {
            gate_type = logic_gate::AND;
        }

        graphInfo.insertNode(gate_name, gate_type, container);

        while(fi >> buffer) {
            if(buffer == ".names") {
                break;
            }
            else if(buffer == ".end") {
                isEnd = true;
            }
        }
    }    
    fi.close();

    return graphInfo;
}