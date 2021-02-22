#include "CPU.hpp"
#include "CPUStat.hpp"
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <bitset>
class Instruction;
using namespace std;


int main (int argc, char* argv[])
{

	//Make sure argument passed is appropriate
	if (argv[1] == NULL) {
		cout << "bad argument!\n";
		exit(EXIT_FAILURE);
	}

	int only_stat_printing = 0;
	if (argv[2] == NULL) {
		only_stat_printing = 1;
	}

	//open file for reading
	const string filename = argv[1];
	ifstream infile;
	infile.open(filename);


	//instructions are 32 bits = 4 bytes
	//each line is 1 byte
	//4 lines are one instruction

	//store all instructions in vector
	vector<string> instMem;
	int val, i=0;
	string line;
	vector<char> vect;
	while(getline(infile, line)) {
		instMem.push_back(line);
	}


	//Instantiate CPU class object, alongside Program Counter (PC) variable and curr address
	CPU myCPU;
	int PC = 0;
	int addr = 0;
	string instr;

	//Instantiate CPUStat object to be ready to record 
	CPUStat stat;

	//condition that when set to false, breaks out of loop below
	bool decoded = true;

	int data[1] = {0};

	//iterate through all the instructions
	while (1)
	{
		//fetch instruction
		instr = myCPU.fetch(instMem);

		//decode fetched instruction
		//decode also sets the register file
		//in turn, the set register file calls EX that calls the ALU
		//ALU to update PC and do any calculations, WriteBack if necessary
		decoded = myCPU.decode(instr, stat);
		if (decoded == false) {
			break;
		}

	}


	//print results
	stat.printStat();

	if (only_stat_printing == 0) {
		//print registers
		if (string(argv[2]) == "") {
			//print only stats
		}
		else if (string(argv[2]) == "-d1") {
			//print reg values
			myCPU.printD1();
		}
		else if (string(argv[2]) == "-d2") {
			//print mem values
			myCPU.printD2();
		}
		else if (string(argv[2]) == "-d3") {
			//print d3 style (both reg file and mem)
			myCPU.printD3();
		}
	}


	//free vector memory
	vector<string>().swap(instMem);

	return 0; 
}


