#ifndef INSTRUCTION_h
#define INSTRUCTION_h
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

class Instruction {  
     public:
        //Constructor to initialize all variables to 0
        Instruction() { 
                rd[0] = rd[1] = rd[2] = rd[3] = rd[4] = 0;
                rs1[0] = rs1[1] = rs1[2] = rs1[3] = rs1[4] = 0;
                rs2[0] = rs2[1] = rs2[2] = rs2[3] = rs2[4] = 0;
                imm[0] = imm[1] = imm[2] = imm[3] = imm[4] = imm[5] = imm[6] = imm[7] = imm[8] = imm[9] = imm[10] = imm[11] = 0;
                funct3[0] = funct3[1] = funct3[2] = 0;
                funct7[0] = funct7[1] = funct7[2] = funct7[3] = funct7[4] = funct7[5] = funct7[6] = 0;
                // type = '';
                opcode[0] = opcode[1] = opcode[2] = opcode[3] = opcode[4] = opcode[5] = opcode[6] = 0;
        }
    
        char type; 
        int opcode[7]; 
        int rd[5]; 
        int funct3[3];
        int rs1[5]; 
        int rs2[5]; 
        int imm[12]; 
        int funct7[7]; 
};

#endif
