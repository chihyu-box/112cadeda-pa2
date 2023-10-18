#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "logic_gate.hpp"

using namespace std;

class Node 
{
private:
	string id;
    logic_gate type;
    int fanin_count;
    int original_fanin_count;
    int fanout_count;
public:
    Node(string id, logic_gate type) : id(id), type(type), fanin_count(0), fanout_count(0) {}
    Node() {}
    string get_id() const {
    	return id;
    }
    void set_type(logic_gate gate_type) {
        type = gate_type;
    }
    logic_gate get_type() const {
        return type;
    }
    void increase_fanin() {
        fanin_count += 1;
    }
    void decrease_fanin() {
        fanin_count -= 1;
    }
    void increase_fanout() {
        fanout_count += 1;
    }
    void decrease_fanout() {
        fanout_count -= 1;
    }
    int get_fanin_count() const {
        return fanin_count;
    }
    int get_fanout_count() const {
        return fanout_count;
    }
    bool fanin_ready() {
        return fanin_count == 0;        
    }
    bool fanout_ready() {
        return fanout_count == 0;
    }
    int get_fanin() {
        return fanin_count;
    }
    int get_fanout() {
        return fanout_count;
    }
    void save_fanin_count() {
        original_fanin_count = fanin_count;
    }
    void restore_fanin_count() {
        fanin_count = original_fanin_count;
    }
};
 

