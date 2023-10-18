#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <memory>
#include "parser.hpp"
#include "Method.hpp"

using namespace std;

int main(int argc, char** argv)
{
	// Set timer
	auto start = chrono::high_resolution_clock::now();
	
	// ========== Program start ==========
	// Is the input correct?
	if(argc != 6) {
		cerr << "Usage : mlrcs <-h/-e> <BLIF_FILE> <AND_CONSTRAINT> <OR_CONSTRAINT> <NOT_CONSTRAINT>" << endl;
		return(1);
	}
	// DEFINE VARIABLE
	fstream fi(argv[2], ios::in);
	string methodArg = argv[1];
	GraphInfo graphInfo;
	int latency = 0;

	// PARSE DATA
	graphInfo = parser(fi);
	fi.close();

	// COMPLETE DATA
	graphInfo.computeNodeDepth();
	graphInfo.computeASAP();
	graphInfo.completeNodeLabel();
	graphInfo.computeTypeNum();
// graphInfo.computeASAP();
// graphInfo.computeALAP(latency+1);
// graphInfo.printALAPLevel(latency+1);
// graphInfo.printGraph();
// graphInfo.printNodeMap();
// graphInfo.printNodeLabel();
// graphInfo.printTypeNum();

	// INTIAIL METHOD
	Method method(graphInfo, stoi(argv[3]), stoi(argv[4]), stoi(argv[5]));
	// SELECT METHOD
	if(methodArg == "-h") {
		latency = method.run_MLRCS();
	}
	else if(methodArg == "-e") {
		method.run_ASAP();
		latency = method.run_MLRCS();
		method.run_ALAP(latency+1);
		method.run_GUROBI(latency+1);
	}
	else if(methodArg == "-glpk") {
		method.run_ASAP();
		latency = method.run_MLRCS();
		method.run_ALAP(latency+1);
		method.run_ILP(latency+1);
	}
	// ========== Program end ==========

	// Stop timer 
	auto stop = chrono::high_resolution_clock::now();

	// Run time
	auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
	cout << "Program end " << duration.count() << " ms." << endl;

	return 0;
}