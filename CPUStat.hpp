#ifndef CPUSTAT_h
#define CPUSTAT_h
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <bitset>
using namespace std;

//Class to keep count of and display counter variables

class CPUStat {
    public:
        //Constructor to initialize all variables to 0
        CPUStat() {
            fetch_instr = R=I=S=B=U=J=SW=LW=ADD=BEQ=Taken=0;
        }

        //variables that act as counters for their respective instructions
        int fetch_instr;
        int R,I,S,B,U,J;
        int SW, LW, ADD, BEQ, Taken;       

        //old print function kept to test if changes made ruin output when tested with CA1 trace files
        void printStatOld() {
            cout << fetch_instr << "\n";
            cout << R << ", " << I << ", " << S << ", " << B << ", " << U << ", " << J << "\n";
            cout << SW << ", " << LW << ", " << ADD << "\n";
        }

        //print required final counters to terminal
        void printStat() {
            cout << fetch_instr << "\n";
            cout << SW << ", " << LW << ", " << ADD << ", " << BEQ << "\n";
            cout << Taken << "\n";
        }


};


#endif