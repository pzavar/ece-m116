#ifndef _gbn_h
#define _gbn_h
#include "Instruction.hpp"
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
using namespace std;

class CPU {  
    public:
        //Constructor to intiailize PC to 0 every time a CPU object is created
        CPU() {
            PC = 0;
            RegWrite=AluSrc=Branch=MemRe=MemWr=MemtoReg=0;
            ALUOp[0]=ALUOp[1]=ALUOp[2]=ALUOp[3]=0;
            regFile[0]=regFile[1]=regFile[2]=regFile[3]=regFile[4]=regFile[5]=regFile[6]=regFile[7]=regFile[8]=regFile[9]=0;
            regFile[10]=regFile[11]=regFile[12]=regFile[13]=regFile[14]=regFile[15]=regFile[16]=regFile[17]=regFile[18]=regFile[19]=0;
            regFile[20]=regFile[21]=regFile[22]=regFile[23]=regFile[24]=regFile[25]=regFile[26]=regFile[27]=regFile[28]=regFile[29]=0;
            regFile[30]=regFile[31]=0;
            Zero = 0;
            memory[0]=memory[1]=memory[2]=memory[3]=memory[4]=memory[5]=memory[6]=memory[7]=0;
        }

        //Fetch declaration
        string fetch(vector <string> instMem);

        //Decode declaration
        bool decode(string instr, CPUStat& stat);

        //Instantiate an object of Instruction to use in Decode function
        Instruction instruction;

        //Controller declaration - create all the control signals as per L7 Slide 12
        char controller(string instr);

        //take addr of reg1, reg2, go to Register File and find values at those addresses
        void setRegisterFile(char type, int reg1_index, int reg2_index, int write_reg_index, string instr, string op, bool RegWrite, CPUStat& stat);

        //EX - call ALU
        //note: default value of imm=0
        //if val2 == 0, we dont need it and it's an I-type
        void EX(int val1, int val2, string instr, char type, string op, CPUStat& stat ,int imm = 0);

        //ALU - doing calculations based on the instruction inputted
        void ALU(int val, int val2, string op, string instr, CPUStat& stat, int imm=0);

        //write in WB function (if at all needed)
        void WB(int res, string op, string instr, int imm);

        //print register file values (x0 through x31) with -d1
        void printD1();

        //print contents of memory with -d2
        void printD2();

        //print both register and memory contents
        void printD3();

        //PCAddr updater (for both +4 or +offset updates)
        int updatePC(int PC, int offset);


    private:
        int PC;
        bool RegWrite, AluSrc, Branch, MemRe, MemWr, MemtoReg, Zero;
        int ALUOp[4];

        //Register File
        int regFile[32];

        //Memory
        //each cell is 8 bytes
        int memory[8];

        string op;

};

#endif