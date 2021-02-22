#include "CPU.hpp"
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
#include <math.h>
using namespace std;

//signal indicating when it's time to break out of main loop
bool end_now = false;

//function to update PC, taking branching into consideration
int CPU::updatePC(int PC, int offset) {
    if (offset == 0) {
        PC += 4;
    }
    else {
        PC += offset;
    }
    return PC;
}

// Set control signals 
char CPU::controller(string instr) {
    Instruction instruction;

    // extract opcode
    int x=0;
    for (int i=25; i<=31; i++) {
        instruction.opcode[x] = stoi(instr.substr(i,1));
        x++;
    }

    // R-type control signals
    if (instruction.opcode[0]==0 && instruction.opcode[1]==1 && instruction.opcode[2]==1 && instruction.opcode[3]==0 
        && instruction.opcode[4]==0 && instruction.opcode[5]==1 && instruction.opcode[6]==1) 
        {
            RegWrite = 1;
            AluSrc   = 0;
            Branch   = 0;
            MemRe    = 0;
            MemWr    = 0;
            MemtoReg = 0;
            instruction.type = 'R';
        }

    // I-type control signals (except LW)
    if (instruction.opcode[0]==0 && instruction.opcode[1]==0 && instruction.opcode[2]==1 && instruction.opcode[3]==0 
        && instruction.opcode[4]==0 && instruction.opcode[5]==1 && instruction.opcode[6]==1) 
        {
            RegWrite = 1;
            AluSrc   = 1;
            Branch   = 0;
            MemRe    = 0;
            MemWr    = 0;
            MemtoReg = 0;
            instruction.type = 'I';
        }

    // LW control signals
    if (instruction.opcode[0]==0 && instruction.opcode[1]==0 && instruction.opcode[2]==0 && instruction.opcode[3]==0 
        && instruction.opcode[4]==0 && instruction.opcode[5]==1 && instruction.opcode[6]==1) 
        {
            RegWrite = 1;
            AluSrc   = 1;
            Branch   = 0;
            MemRe    = 1;
            MemWr    = 0;
            MemtoReg = 1;
            instruction.type = 'I';
        }

    // SW control signals
    if (instruction.opcode[0]==0 && instruction.opcode[1]==1 && instruction.opcode[2]==0 && instruction.opcode[3]==0 
        && instruction.opcode[4]==0 && instruction.opcode[5]==1 && instruction.opcode[6]==1) 
        {
            RegWrite = 0;
            AluSrc   = 1;
            Branch   = 0;
            MemRe    = 0;
            MemWr    = 1;
            MemtoReg = 0;
            instruction.type = 'S';
        }

    // BEQ control signals
    if (instruction.opcode[0]==1 && instruction.opcode[1]==1 && instruction.opcode[2]==0 && instruction.opcode[3]==0 
        && instruction.opcode[4]==0 && instruction.opcode[5]==1 && instruction.opcode[6]==1) 
        {
            RegWrite = 0;
            AluSrc   = 0;
            Branch   = 1;
            MemRe    = 0;
            MemWr    = 0;
            MemtoReg = 0;
            instruction.type = 'B';
        }

    return instruction.type;
}


//fetch the specific instruction from the entire instructions stored in instMem vector
string CPU::fetch(vector<string> instMem) {
    //Set flag to know this instruction is the final instruction in instMem
    if (&instMem[PC+3] == &instMem.back()) {
        end_now = true;
    }

    //store the values in string variables to make it easier to work with
    string str_byte1 = instMem[PC];
    string str_byte2 = instMem[PC+1];
    string str_byte3 = instMem[PC+2];
    string str_byte4 = instMem[PC+3];

    //convert the strings into integers
    int i_byte1 = stoi(str_byte1);
    int i_byte2 = stoi(str_byte2);
    int i_byte3 = stoi(str_byte3);
    int i_byte4 = stoi(str_byte4);

    //store them as int binary
    bitset<8> bitset_byte1(i_byte1);
    bitset<8> bitset_byte2(i_byte2);
    bitset<8> bitset_byte3(i_byte3);
    bitset<8> bitset_byte4(i_byte4);
 
    //convert back to string
    string res_byte1 = bitset_byte1.to_string();
    string res_byte2 = bitset_byte2.to_string();
    string res_byte3 = bitset_byte3.to_string();
    string res_byte4 = bitset_byte4.to_string();

    //concatenate to create full 32-bit instruction
    string str_instr = res_byte4 + res_byte3 + res_byte2 + res_byte1;

    //convert string into bits
    bitset<32> bitset_instr(str_instr);
    
    //return fetched instruction
    return str_instr;
}

//take in the instruction, and decode it
bool CPU::decode(string instr, CPUStat& stat) {
    Instruction instruction;

    Zero = 0;

    //extract opcode
    int x=0;
    for (int i=25; i<=31; i++) {
        instruction.opcode[x] = stoi(instr.substr(i,1));
        x++;
    } //store in opcode[7] for easier use
    int opcode[7];
    opcode[0] = { instruction.opcode[0] };
    opcode[1] = { instruction.opcode[1] };
    opcode[2] = { instruction.opcode[2] };
    opcode[3] = { instruction.opcode[3] };
    opcode[4] = { instruction.opcode[4] };
    opcode[5] = { instruction.opcode[5] };
    opcode[6] = { instruction.opcode[6] };

    //condition to break out of loop
    if (instruction.opcode[0]==0 && instruction.opcode[1]==0 && instruction.opcode[2]==0 && instruction.opcode[3]==0 && instruction.opcode[4]==0 && instruction.opcode[5]==0 && instruction.opcode[6]==0) {
        cout << "Exit....\n";
        return false;
    }

    //Set controller values and return type
    char type;
    type = controller(instr);

    //extract funct3
    int j=0;
    for (int i=17; i<=19; i++) {
        instruction.funct3[j] = stoi(instr.substr(i,1));
        j++;
    }
    //store extracted funct3 values into funct3 array for easier use
    int funct3[3];
    funct3[0] = { instruction.funct3[0] };
    funct3[1] = { instruction.funct3[1] };
    funct3[2] = { instruction.funct3[2] };


    //extract rs1
    j=0;
    for (int i=12; i<=16; i++) {
        instruction.rs1[j] = stoi(instr.substr(i,1));
        j++;
    }

    //extract rs2
    j=0;
    for (int i=7; i<= 11; i++) {
        instruction.rs2[j] = stoi(instr.substr(i,1));
        j++;
    }

    //extract rd
    j=0; 
    for (int i=20; i<=24; i++) {
        instruction.rd[j] = stoi(instr.substr(i,1));
        j++;
    }


    // if type='R'
    // Increment CPUStat variables
    if (type == 'R') {
        //extract funct7
        j=0;
        for (int i=0; i<=6; i++) {
            instruction.funct7[j] = stoi(instr.substr(i,1));
            j++;
        }
        //store extracted funct7 values into funct7 array for easier use
        int funct7[7];
        funct7[0] = { instruction.funct7[0] };
        funct7[1] = { instruction.funct7[1] };
        funct7[2] = { instruction.funct7[2] };
        funct7[3] = { instruction.funct7[3] };
        funct7[4] = { instruction.funct7[4] };
        funct7[5] = { instruction.funct7[5] };
        funct7[6] = { instruction.funct7[6] };

        //double checking that type and opcode are consistent with one another
        if (opcode[0]==0 && opcode[1]==1 && opcode[2]==1 && opcode[3]==0 && opcode[4]==0 && opcode[5]==1 && opcode[6]==1) {
            //ADD
            if (funct3[0]==0 && funct3[1]==0 && funct3[2]==0  && funct7[1]==0) {
                stat.ADD += 1;
                stat.fetch_instr += 1;
                stat.R += 1;

                ALUOp[0] = 0;
                ALUOp[1] = 0;
                ALUOp[2] = 1;
                ALUOp[3] = 0;

                op = "add";
            }

            //SUB
            else if (funct3[0]==0 && funct3[1]==0 && funct3[2]==0  && funct7[1]==1) {
                stat.fetch_instr += 1;
                stat.R += 1;

                ALUOp[0] = 0;
                ALUOp[1] = 1;
                ALUOp[2] = 1;
                ALUOp[3] = 0;

                op = "sub";
            } 

            //OR
            else if (funct3[0]==1 && funct3[1]==1 && funct3[2]==0) {
                stat.fetch_instr += 1;
                stat.R += 1;

                ALUOp[0] = 0;
                ALUOp[1] = 0;
                ALUOp[2] = 0;
                ALUOp[3] = 1;

                op = "or";
            }

            //AND
            else if (funct3[0]==1 && funct3[1]==1 && funct3[2]==1) {
                stat.fetch_instr += 1;
                stat.R += 1;
                
                ALUOp[0] = 0;
                ALUOp[1] = 0;
                ALUOp[2] = 0;
                ALUOp[3] = 0;

                op = "and";
            }
            else {
                cout <<"ERROR: Did not receive any R-type instruction\n";
                exit(EXIT_FAILURE);
            }
        }
    }

    //if type='I'
    //need to also extract imm[11:0] and rd
    if (type == 'I') {
        j=0;
        for (int i=0; i<=11; i++) {
            instruction.imm[j] = stoi(instr.substr(i,1));
            j++;
        }

        //I-types don't need rs2...
        instruction.rs2[0] = instruction.rs2[1] = instruction.rs2[2] = instruction.rs2[3] = instruction.rs2[4] = 0;
    }

    //if type='S'
    if (type == 'S') {
        // Already have: rs1, funct3, opcode
        // Need: imm, rs2

        instruction.rd[0]=instruction.rd[1]=instruction.rd[2]=instruction.rd[3]=instruction.rd[4]=0;

        //imm:
        // - imm[4:0]  : bits [20 - 24]
        // - imm[11:5] : bits [0 - 6]
        j=0;
        for (int i=20; i<=24; i++) {
            instruction.imm[j] = stoi(instr.substr(i,1));
            j++;
        }
        j=5;
        for (int i=0; i<=6; i++) {
            instruction.imm[j] = stoi(instr.substr(i,1));
            j++;
        }
    }

    //if type='B'
    if (type == 'B') {
        //B-types don't need rd either
        instruction.rd[0]=instruction.rd[1]=instruction.rd[2]=instruction.rd[3]=instruction.rd[4]=0;
    } 


    //If ADDI, ORI, ANDI (I-type) - set ALUOp values
    if (opcode[0]==0  &&  opcode[1]==0  &&  opcode[2]==1  &&  opcode[3]==0) {
        stat.fetch_instr += 1;
        stat.I += 1;

        //addi
        if (funct3[0]==0 && funct3[1]==0 && funct3[2]==0) {
            ALUOp[0] = 0;
            ALUOp[1] = 0;
            ALUOp[2] = 1;
            ALUOp[3] = 0;
            op = "addi";
        }

        //ori
        if (funct3[0]==1 && funct3[1]==1 && funct3[2]==0) {
            ALUOp[0] = 0;
            ALUOp[1] = 0;
            ALUOp[2] = 0;
            ALUOp[3] = 1;
            op = "ori";
        }

        //andi
        if (funct3[0]==1 && funct3[1]==1 && funct3[2]==1) {
            ALUOp[0] = 0;
            ALUOp[1] = 0;
            ALUOp[2] = 0;
            ALUOp[3] = 0;
            op = "andi";
        }
    }

    //If LW (I-type)
    if (opcode[0]==0  &&  opcode[1]==0  &&  opcode[2]==0  &&  opcode[3]==0) {
        // cout << "in LW\n";
        stat.fetch_instr += 1;
        stat.LW += 1;
        stat.I += 1;
        
        ALUOp[0] = 0;
        ALUOp[1] = 0;
        ALUOp[2] = 1;
        ALUOp[3] = 0;
        op = "lw";
    }

    //If SW (S-type)
    if (opcode[0]==0  &&  opcode[1]==1  &&  opcode[2]==0  &&  opcode[3]==0) {
        // cout << "in SW\n";
        stat.fetch_instr += 1;
        stat.SW += 1;
        stat.S += 1;
        
        ALUOp[0] = 0;
        ALUOp[1] = 0;
        ALUOp[2] = 1;
        ALUOp[3] = 0;
        op = "sw";
    }

    //If BEQ (B-type)
    if (opcode[0]==1  &&  opcode[1]==1  &&  opcode[2]==0) {
        // cout << "in BEQ\n";
        stat.fetch_instr += 1;
        stat.B += 1;
        
        ALUOp[0] = 0;
        ALUOp[1] = 1;
        ALUOp[2] = 1;
        ALUOp[3] = 0;
        stat.BEQ++;
        op = "beq";
    }

    //Convert values of rs1, rs2, rd to decimal int
    //rs1
    int rs1_addr = 0;
    for (int i=0; i<=4; i++) {
        if (instruction.rs1[i] == 1) {
            rs1_addr += pow(2, 4-i);
        }
    }
    //rs2
    int rs2_addr = 0;
    for (int i=0; i<=4; i++) {
        if (instruction.rs2[i] == 1) {
            rs2_addr += pow(2, 4-i);
        }
    }
    //rd
    int rd_addr = 0;
    for (int i=0; i<=4; i++) {
        if (instruction.rd[i] == 1) {
            rd_addr += pow(2, 4-i);
        }
    }

    //set regFile by calling function
    setRegisterFile(type, rs1_addr, rs2_addr, rd_addr, instr, op, RegWrite, stat);
    
    //condition to break out of while loop
    if (end_now == true) {
        return false;
    }

    //if we get here, there are more instructions to be read!
    return true;
}



void CPU::setRegisterFile(char type, int reg1_index, int reg2_index, int write_reg_index, string instr, string op, bool RegWrite, CPUStat& stat) {
    
    //if ALUSrc=0, we need to read data 2 
    //and feed read_data_1 and read_data_2 into EX()
    //EITHER R-TYPE OR B-TYPE
    if (AluSrc == 0) {
        //extract contents using register indexes
        int reg1 = regFile[reg1_index];
        int reg2 = regFile[reg2_index];

        //extract imm
        if (type == 'B') {
            int imm = 0;
            int instr_imm_arr[13] = { 0 };
            instr_imm_arr[0] = stoi(instr.substr(0,1));
            instr_imm_arr[1] = stoi(instr.substr(24,1));
            instr_imm_arr[2] = stoi(instr.substr(1,1));
            instr_imm_arr[3] = stoi(instr.substr(2,1));
            instr_imm_arr[4] = stoi(instr.substr(3,1));
            instr_imm_arr[5] = stoi(instr.substr(4,1));
            instr_imm_arr[6] = stoi(instr.substr(5,1));
            instr_imm_arr[7] = stoi(instr.substr(6,1));
            instr_imm_arr[8] = stoi(instr.substr(20,1));
            instr_imm_arr[9] = stoi(instr.substr(21,1));
            instr_imm_arr[10] = stoi(instr.substr(22,1));
            instr_imm_arr[11] = stoi(instr.substr(23,1));

            //handling 2's complement sign extension
            int neg = 0;
            if (instr_imm_arr[0] == 1) {
                for (int i=0; i<13; i++) {
                    //flip the bits
                    if (instr_imm_arr[i] == 1) {
                        instr_imm_arr[i] = 0;
                    }
                    else if (instr_imm_arr[i] == 0) {
                        instr_imm_arr[i] = 1;
                    }
                }

                neg = 1;
            }
            for (int i=1; i<=12; i++) {
                if (instr_imm_arr[i] == 1) {
                    imm += pow(2, 12-i);
                }
            }

            if (neg == 1) {
                imm++;
                imm *= -1;
            }

            EX(reg1, reg2, instr, type, op, stat, imm);
        }

        else if (type == 'R') {
            int rd = regFile[write_reg_index];
            EX(reg1, reg2, instr, type, op, stat, 0);
        }

    }

    //if 1, we don't read data 2
    //instead, we need to extract the immediate value
    //and feed read_data_1 and imm into EX
    else if (AluSrc == 1) {
        int reg1 = regFile[reg1_index];
        int imm = 0;
        
        if (type == 'I') {
            int j=0;
            for (int i=0; i<=11; i++) {
                instruction.imm[j] = stoi(instr.substr(i,1));
                j++;
            }
            
            //handling 2's complement sign extension
            int neg=0;
            if (instruction.imm[0] == 1) {
                for (int i=0; i<12; i++) {
                    if (instruction.imm[i] == 1) {
                        instruction.imm[i] = 0;
                    } else if (instruction.imm[i] == 0) {
                        instruction.imm[i] = 1;
                    }
                }
                neg = 1;
            }

            for (int i=1; i<=11; i++) {
                if (instruction.imm[i] == 1) {
                    imm += pow(2, 11-i);
                }
            }

            if (neg == 1) {
                imm++;
                imm *= -1;
            }

            EX(reg1, 0, instr, type, op, stat, imm);
        }

        else if (type == 'S') {
            int reg2 = regFile[reg2_index];
            int j=7;
            for (int i=20; i<=24; i++) {
                instruction.imm[j] = stoi(instr.substr(i,1));
                j++;
            }
            j=0;
            for (int i=0; i<=6; i++) {
                instruction.imm[j] = stoi(instr.substr(i,1));
                j++;
            }

            //handling 2's complement sign extension
            int neg = 0;
            if (instruction.imm[0] == 1) {
                for (int i=0; i<12; i++) {
                    if (instruction.imm[i] == 1) {
                        instruction.imm[i] = 0;
                    } else if (instruction.imm[i] == 0) {
                        instruction.imm[i] = 1;
                    }
                }
                neg = 1;
            }

            for (int i=1; i<=11; i++) {
                if (instruction.imm[i] == 1) {
                    imm += pow(2, 11-i);
                }
            }

            if (neg == 1) {
                imm++;
                imm *= -1;
            }

            EX(reg1, reg2, instr, type, op, stat, imm);
        }
    }
}

//Higher function above ALU passing appropriate values into ALU per instruction
void CPU::EX(int val1, int val2, string instr, char type, string op, CPUStat& stat, int imm) {
    if (imm == 0) {
        //no immediate exists for current instruction ==> R-type
        //call ALU without imm (imm is a default parameter. See declaration for details)
        if (type == 'R') {
            ALU(val1, val2, op, instr,stat);
        }
    }
    if (val2 == 0) {
        //no reg2 exists for current instruction ==> I-type
        //call ALU without reg2 
        if (type == 'I') {
            ALU(val1, imm, op, instr, stat, imm);
        }
    }

    if (type == 'S') {
        ALU(val1, val2, op, instr, stat, imm);
    }

    if (type == 'B') {
        ALU(val1, val2, op, instr, stat, imm);
    }
}

//ALU - where all the addings, subtracting, etc take place
//PC is also updated here
//also, we call the WB function that handles writing into our registers/memory
void CPU::ALU(int reg1_data, int reg2_data, string op, string instr, CPUStat& stat, int imm) {
    //R-type
    int ALU_result = 0;
    if (op == "add") {
        ALU_result = reg1_data + reg2_data;
        if (RegWrite == 1) {
            WB(ALU_result, op, instr, imm);
        }
        PC = updatePC(PC, 0);
    }
    if (op == "sub") {
        ALU_result = reg1_data - reg2_data;
        if (ALU_result == 0) { Zero = 1; }
        if (RegWrite == 1) {
            WB(ALU_result, op, instr, imm);
        }
        PC = updatePC(PC, 0);
    }
    if (op == "or") {
        ALU_result = reg1_data | reg2_data;
        if (RegWrite == 1) {
            WB(ALU_result, op, instr, imm);
        }
        PC = updatePC(PC, 0);
    }
    if (op == "and") {
        ALU_result = reg1_data & reg2_data;
        if (RegWrite == 1) {
            WB(ALU_result, op, instr, imm);
        }
        PC = updatePC(PC, 0);
    }

    //I-type
    if (op == "addi") {
        // cout << "reg1_data:\t" << reg1_data << "\n";
        // cout << "imm      :\t" << imm << "\n";
        ALU_result = reg1_data;
        ALU_result += imm;
        // cout << "ALU_res:  \t" << ALU_result << "\n";
        if (RegWrite == 1) {
            WB(ALU_result, op, instr, imm);
        }
        PC = updatePC(PC, 0);
    }
    if (op == "ori") {
        ALU_result = reg1_data | imm;
        if (RegWrite == 1) {
            WB(ALU_result, op, instr, imm);
        }
        PC = updatePC(PC, 0);
    }
    if (op == "andi") {
        ALU_result = reg1_data & imm;
        if (RegWrite == 1) {
            WB(ALU_result, op, instr, imm);
        }
        PC = updatePC(PC, 0);
    }

    //LW
    if (op == "lw" && MemRe == 1) {
        //extract rs1 and rd
        int j=0;
        for (int i=12; i<=16; i++) {
            instruction.rs1[j] = stoi(instr.substr(i,1));
            j++;
        }
        j=0; 
        for (int i=20; i<=24; i++) {
            instruction.rd[j] = stoi(instr.substr(i,1));
            j++;
        }
        int rs1_addr = 0;
        for (int i=0; i<=4; i++) {
            if (instruction.rs1[i] == 1) {
                rs1_addr += pow(2, 4-i);
            }
        }

        //increment rs1 by imm/4 to get needed mem_addr
        int mem_addr = rs1_addr / 4;
        int mem_imm = imm / 4;
        mem_addr += mem_imm;
        // cout << "mem addr: " << mem_addr << "\n";

        int ALU_result = memory[mem_addr];
        if (RegWrite == 1) {
            WB(ALU_result, op, instr, imm);
        }
        
        //go to memory with needed address, take contents
        // regFile[rd_addr] = memory[added_addr]; 

        //store those contents into rd
        PC = updatePC(PC, 0);
    }
    if (op == "sw" && MemWr == 1) {
        //extract rs1, rs2
        int j=0;
        for (int i=12; i<=16; i++) {
            instruction.rs1[j] = stoi(instr.substr(i,1));
            j++;
        }
        j=0;
        for (int i=7; i<= 11; i++) {
            instruction.rs2[j] = stoi(instr.substr(i,1));
            j++;
        }
        int rs1_addr = 0;
        for (int i=0; i<=4; i++) {
            if (instruction.rs1[i] == 1) {
                rs1_addr += pow(2, 4-i);
            }
        }
        int rs2_addr = 0;
        for (int i=0; i<=4; i++) {
            if (instruction.rs2[i] == 1) {
                rs2_addr += pow(2, 4-i);
            }
        }

        int mem_dest_addr = imm + rs1_addr;
        mem_dest_addr /= 4;
        int rs2_data = regFile[rs2_addr];

        if (MemWr == 1) {
            WB(rs2_data, op, instr, mem_dest_addr);
        }
        PC = updatePC(PC, 0);
    }

    //BEQ
    if (op == "beq") {
        int res = reg1_data - reg2_data;
        Zero = 0;
        if (res == 0) {
            Zero = 1;
        }
        if (Branch == 1 && Zero == 1) {
            // cout << "Branching...\n";
            Zero = 0;
            PC = updatePC(PC, imm);
            stat.Taken++;
        }
        else if (Zero == 0) {
            PC = updatePC(PC, 0);
            // cout << "BEQ'd but didn't branch\n";
        }
    }
}

//Writing to registers/memory depending on instruction
void CPU::WB(int res, string op, string instr, int imm) {

    if (op != "sw" && op != "beq" ) {
        //extract destination address
        int j=0; 
        for (int i=20; i<=24; i++) {
            instruction.rd[j] = stoi(instr.substr(i,1));
            j++;
        }
        //convert binary rd addr to decimal rd addr
        int rd_addr=0;
        for (int i=0; i<=4; i++) {
            if (instruction.rd[i] == 1) {
                rd_addr += pow(2, 4-i);
            }
        }

        regFile[rd_addr] = res;
    }

    if (op == "sw" && MemWr == 1) {
        //destination addr: res (i.e. rs1+imm)
        //source: rs2
        int mem_dest_addr = imm;
        int reg_data = res;       
        
        memory[mem_dest_addr] = reg_data;
    }
    
}

//Print contents of registers
void CPU::printD1() {
    cout << "\n/****** START OF REGISTER PRINT ******/\n";
    cout << "x0:  " << regFile[0]  << "\t\t" << "x1:  " << regFile[1]  << "\n";
    cout << "x2:  " << regFile[2]  << "\t\t" << "x3:  " << regFile[3]  << "\n";
    cout << "x4:  " << regFile[4]  << "\t\t" << "x5:  " << regFile[5]  << "\n";
    cout << "x6:  " << regFile[6]  << "\t\t" << "x7:  " << regFile[7]  << "\n";
    cout << "x8:  " << regFile[8]  << "\t\t" << "x9:  " << regFile[9]  << "\n";
    cout << "x10: " << regFile[10] << "\t\t" << "x11: " << regFile[11] << "\n";
    cout << "x12: " << regFile[12] << "\t\t" << "x13: " << regFile[13] << "\n";
    cout << "x14: " << regFile[14] << "\t\t" << "x15: " << regFile[15] << "\n";
    cout << "x16: " << regFile[16] << "\t\t" << "x17: " << regFile[17] << "\n";
    cout << "x18: " << regFile[18] << "\t\t" << "x19: " << regFile[19] << "\n";
    cout << "x20: " << regFile[20] << "\t\t" << "x21: " << regFile[21] << "\n";
    cout << "x22: " << regFile[22] << "\t\t" << "x23: " << regFile[23] << "\n";
    cout << "x24: " << regFile[24] << "\t\t" << "x25: " << regFile[25] << "\n";
    cout << "x26: " << regFile[26] << "\t\t" << "x27: " << regFile[27] << "\n";
    cout << "x28: " << regFile[28] << "\t\t" << "x29: " << regFile[29] << "\n";
    cout << "x30: " << regFile[30] << "\t\t" << "x31: " << regFile[31] << "\n";
    cout << "/****** END OF REGISTER PRINT ******/\n\n";
}

//print contents of memory
void CPU::printD2() {
    cout << "\n/****** START OF MEMORY PRINT ******/\n";
    cout << "0 (0x00): " << memory[0] << "\t\t"  << "01 (0x04): " << memory[1]   << "\n";
    cout << "2 (0x08): " << memory[2] << "\t\t"  << "03 (0x0C): " << memory[3]   << "\n";
    cout << "4 (0x10): " << memory[4] << "\t\t"  << "05 (0x14): " << memory[5]   << "\n";
    cout << "6 (0x18): " << memory[6] << "\t\t"  << "07 (0x1C): " << memory[7]   << "\n";
    cout << "/****** END OF MEMORY PRINT ******/\n\n";

}

//bring both contents of registers and memory
void CPU::printD3() {
    printD1();
    printD2();
}