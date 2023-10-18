#pragma once

#include "GraphInfo.hpp"
#include "ML_RCS.hpp"
#include "ILP.hpp"
#include "GUROBI.hpp"
using namespace std;

class Method {
private:
    GraphInfo &graphInfo;
    int AND_num;
    int OR_num;
    int NOT_num;
public:
    Method(GraphInfo &graphInfo, int AND_num, int OR_num, int NOT_num)
    : graphInfo(graphInfo), AND_num(AND_num), OR_num(OR_num), NOT_num(NOT_num) {}

    int run_MLRCS() {
        ML_RCS mlrcs(graphInfo, AND_num, OR_num, NOT_num);
        mlrcs.execute();
        mlrcs.write_out();
        return mlrcs.get_latency();
    }

    void run_ASAP() {
        graphInfo.computeASAP();
        // graphInfo.printASAPLevel();
    }

    void run_ALAP(int latency) {
        graphInfo.computeALAP(latency);    
        // graphInfo.printALAPLevel(latency);
    }
    
    int run_ILP(int latency) {
    	ILP ilp(graphInfo, latency, AND_num, OR_num, NOT_num);
    	ilp.solve();
    	ilp.write_out();
    	return ilp.get_latency();
    }

    int run_GUROBI(int latency) {
        try {
            GUROBI gurobi(graphInfo, latency, AND_num, OR_num, NOT_num);
            gurobi.solve();
            gurobi.write_out();
            return gurobi.get_latency();       
        } catch (const GRBException& e) {
            cerr << "Error code: " << e.getErrorCode() << endl;
            cerr << e.getMessage() << endl;
        } catch (const std::exception& e) {
            cerr << "Standard exception: " << e.what() << endl;
        } catch (...) {
            cerr << "Unknown exception caught!" << endl;
        }
        return -1;
    }
};

