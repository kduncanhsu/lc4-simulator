/*
 * LC4.c: Defines simulator functions for executing instructions
 */

#include "LC4.h"
#include <stdio.h>

#define INSN_OP(I) ((I) >> 12)
#define INSN_PRIV_BIT(I) (((I) >> 15) & 0x1)
#define INSN_11_9(I) (((I) >> 9) & 0x7)
#define INSN_8_6(I) (((I) >> 6) & 0x7)
#define INSN_5_3(I) (((I) >> 3) & 0x7)
#define INSN_2_0(I) (((I) >> 0) & 0x7)
#define INSN_4_0(I) (((I) >> 0) & 0xF)
#define INSN_8_7(I) (((I) >> 7) & 0x3)
#define INSN_5_4(I) (((I) >> 4) & 0x3)
#define INSN_11_0(I) (((I) >> 0) & 0x7FF)
#define INSN_11(I) (((I) >> 11) & 0x1)
#define INSN_IMM5(I) (((I) >> 0) & 0x1F)
#define INSN_IMM7(I) (((I) >> 0) & 0x7F)
#define INSN_IMM9(I) (((I) >> 0) & 0x1FF)
#define INSN_IMM8(I) (((I) >> 0) & 0xFF)
#define INSN_IMM6(I) (((I) >> 0) & 0x3F)

// global variables
unsigned short int instr;
unsigned short int global_Rd;
int error;

// helper function declarations
void STROp(MachineState* CPU, FILE* output);
void LDROp(MachineState* CPU, FILE* output);
void RTIOp(MachineState* CPU, FILE* output);
void ConstOp(MachineState* CPU, FILE* output);
void HiConstOp(MachineState* CPU, FILE* output);
void TrapOp(MachineState* CPU, FILE* output);
signed short int sext(signed short int num, int num_length);
int fprintf_unsigned_to_binary(unsigned short int num, FILE* output);
signed short int unsigned_to_signed_short(unsigned short int num);


/*
 * Reset the machine state as Pennsim would do
 */
void Reset(MachineState* CPU)
{

    int i; 

    CPU -> PC = 0x8200;
    CPU -> PSR = 0x8002; 
    ClearSignals(CPU);
    (CPU -> R)[1] = 0x0000;
    (CPU -> R)[2] = 0x0000;
    (CPU -> R)[3] = 0x0000;
    (CPU -> R)[4] = 0x0000;
    (CPU -> R)[5] = 0x0000;
    (CPU -> R)[6] = 0x0000;
    (CPU -> R)[7] = 0x0000;
    
    for (i = 0; i < 65536; i++) {
		(CPU -> memory)[i] = 0;
	}
     
}


/*
 * Clear all of the control signals (set to 0)
 */
void ClearSignals(MachineState* CPU)
{

    CPU -> rsMux_CTL = 0;
    CPU -> rtMux_CTL = 0;
    CPU -> rdMux_CTL = 0;
    CPU -> regFile_WE = 0;
    CPU -> NZP_WE = 0;
    CPU -> DATA_WE = 0;

    CPU -> NZPVal = 0;
    CPU -> regInputVal = 0;
    CPU -> dmemAddr = 0;
    CPU -> dmemValue = 0;
    global_Rd = 0;

}

/*
 * This function should write out the current state of the CPU to the file output.
 */
void WriteOut(MachineState* CPU, FILE* output)
{
    printf("entered WriteOut with PC: ");
    printf("%04X\n", CPU -> PC);

    // current PC
    fprintf(output, "%04X ", CPU -> PC);

    // current instruction (written in binary)
    fprintf_unsigned_to_binary(instr, output);

    // register file WE
    fprintf(output, " %X ", CPU -> regFile_WE);

    // register being written to
    fprintf(output, "%X ", global_Rd);

    // value being written to the register file
    fprintf(output, "%04X ", CPU -> regInputVal);

    // NZP WE
    fprintf(output, "%X ", CPU -> NZP_WE);

    // NZP bits being set to positive/negative
    fprintf(output, "%X ", CPU -> NZPVal);

    // data WE
    fprintf(output, "%X ", CPU -> DATA_WE);

    // data memory address
    fprintf(output, "%04X ", CPU -> dmemAddr);

    // data value
    fprintf(output, "%04X\n", CPU -> dmemValue);

}


/*
 * This function should execute one LC4 datapath cycle.
 */
int UpdateMachineState(MachineState* CPU, FILE* output)
{

    unsigned short int opcode;

    printf("entered UpdateMachineState\n");

    // checking if we are attempting to execute a data section address as code
    if ((CPU -> PC >= 0x2000 && CPU -> PC <= 0x7FFF) || (CPU -> PC >= 0xA000 && CPU -> PC <= 0xFFFF)) {
        printf("error: illegal operation, cannot execute a data section address as code.\n");
        return -1;
    }

    instr = (CPU -> memory)[CPU -> PC];
    error = 0;

    //printf("current instruction: ");
    //printf("0x%04X\n", instr);
    
    opcode = INSN_OP(instr);

    switch(opcode) {
        case 0x0000 : // binary 0000
            BranchOp(CPU, output);
            break;
        case 0x0001 : // binary 0001
            ArithmeticOp(CPU, output);
            break;
        case 0x0002 : // binary 0010
			ComparativeOp(CPU, output);
            break;
        case 0x0004 : // binary 0100
			JSROp(CPU, output);
            break;
        case 0x0005 : // binary 0101
			LogicalOp(CPU, output);
            break;
        case 0x0006 : // binary 0110
            LDROp(CPU, output);
            if (error == 1) {
                return -1;
            }
            break;
        case 0x0007 : // binary 0111
            STROp(CPU, output);
            if (error == 1) {
                return -1;
            }
            break;
        case 0x0008 : // binary 1000
            RTIOp(CPU, output);
            break;
        case 0x0009 : // binary 1001
            ConstOp(CPU, output);
            break;
        case 0x000A : // binary 1010
			ShiftModOp(CPU, output);
            break;
        case 0x000C : // binary 1100
            JumpOp(CPU, output);
            break;
        case 0x000D : // binary 1101
            HiConstOp(CPU, output);
            break;
        case 0x000F : // binary 1111
            TrapOp(CPU, output);
            break;
        default:
            printf("Error in current instruction.\n");
            return -1;
    }
    printf("PC: ");
    printf("%04X\n", CPU -> PC);

    return 0;
}



//////////////// PARSING HELPER FUNCTIONS ///////////////////////////



/*
 * Parses rest of branch operation and updates state of machine.
 */
void BranchOp(MachineState* CPU, FILE* output)
{

    printf("entered BranchOp.\n");
    unsigned short int sub_opcode = INSN_11_9(instr);
    signed short int imm_nine = INSN_IMM9(instr);

    ClearSignals(CPU);

    // based on PSR value bits[0:2], determine the NZP value (CPU -> NZPVal is always 0 at this point)
    unsigned short int NZPVal_temp = INSN_2_0(CPU -> PSR);

    WriteOut(CPU, output);

    switch(sub_opcode) {
        case 0 : // binary 000 (NOP) - do nothing
            CPU -> PC = CPU -> PC + 1;
            break;
        case 1 : // binary 001 (BRp)
            if (NZPVal_temp == 1) {
                CPU -> PC = CPU -> PC + 1 + sext(imm_nine, 9);
            } else {
                CPU -> PC = CPU -> PC + 1;
            }
            break;
        case 2 : // binary 010 (BRz)
            if (NZPVal_temp == 2) {
                CPU -> PC = CPU -> PC + 1 + sext(imm_nine, 9);
            } else {
                CPU -> PC = CPU -> PC + 1;
            }
            break;
        case 3 : // binary 011 (BRzp)
            if (NZPVal_temp == 1 || NZPVal_temp == 2) {
                CPU -> PC = CPU -> PC + 1 + sext(imm_nine, 9);
            } else {
                CPU -> PC = CPU -> PC + 1;
            }
            break;
        case 4 : // binary 100 (BRn)
            if (NZPVal_temp == 4) {
                CPU -> PC = CPU -> PC + 1 + sext(imm_nine, 9);
            } else {
                CPU -> PC = CPU -> PC + 1;
            }
            break;
        case 5 : // binary 101 (BRnp)
            if (NZPVal_temp == 4 || NZPVal_temp == 1) {
                CPU -> PC = CPU -> PC + 1 + sext(imm_nine, 9);
            } else {
                CPU -> PC = CPU -> PC + 1;
            }
            break;
        case 6 : // binary 110 (BRnz)
            if (NZPVal_temp == 2 || NZPVal_temp == 4) {
                CPU -> PC = CPU -> PC + 1 + sext(imm_nine, 9);
            } else {
                CPU -> PC = CPU -> PC + 1;
            }
            break;
        case 7 : // binary 111 (BRnzp)
            CPU -> PC = CPU -> PC + 1 + sext(imm_nine, 9);
            break;
        default:
            printf("Error while parsing branch instruction.\n");
    }

}

/*
 * Parses rest of arithmetic operation and prints out.
 */
void ArithmeticOp(MachineState* CPU, FILE* output)
{
    printf("entered ArithmeticOp.\n");

    unsigned short int Rd;
    unsigned short int Rs;
    unsigned short int Rt;
    unsigned short int sub_opcode;
    signed short int imm_five;

	unsigned short int Rd_value;

    Rd = INSN_11_9(instr);
    Rs = INSN_8_6(instr);
	Rt = INSN_2_0(instr);
    imm_five = INSN_IMM5(instr);
    sub_opcode = INSN_5_3(instr);
    
    ClearSignals(CPU);
	CPU -> regFile_WE = 1;
	CPU -> NZP_WE = 1;
    CPU -> rsMux_CTL = 0;
    CPU -> rtMux_CTL = 0;
    CPU -> rdMux_CTL = 0;
    global_Rd = Rd;


    if (sub_opcode == 0) { // binary 000 (ADD)
        // Rd = Rs + Rt
        Rd_value = (CPU -> R)[Rs] + (CPU -> R)[Rt];
    } else if (sub_opcode == 1) { // binary 001 (MUL)
		// Rd = Rs * Rt
        Rd_value = (CPU -> R)[Rs] * (CPU -> R)[Rt];
    } else if (sub_opcode == 2) { // binary 010 (SUB)
		// Rd = Rs - Rt
        Rd_value = (CPU -> R)[Rs] - (CPU -> R)[Rt];
    } else if (sub_opcode == 3) { // binary 011 (DIV)
		// Rd = Rs / Rt
        Rd_value = (CPU -> R)[Rs] / (CPU -> R)[Rt];
    } else if (sub_opcode >= 4) { // binary 1XX (ADD IMM)
		Rd_value = (CPU -> R)[Rs] + sext(imm_five, 5);
    } else {
        printf("Error while parsing arithmetic instruction\n");
    }
	(CPU -> R)[Rd] = Rd_value;
	CPU -> regInputVal = Rd_value;
    SetNZP(CPU, Rd_value);
    WriteOut(CPU, output);
    CPU -> PC = CPU -> PC + 1;

}

/*
 * Parses rest of comparative operation and prints out.
 */
void ComparativeOp(MachineState* CPU, FILE* output)
{
	printf("entered ComparativeOp.\n");

    signed short int imm_seven;
    unsigned short int u_imm_seven;
    unsigned short int Rs = INSN_11_9(instr);
    unsigned short int Rt = INSN_2_0(instr);
    unsigned short int value;
    signed short int signed_value;

    unsigned short int sub_opcode = INSN_8_7(instr);

    ClearSignals(CPU);
    CPU -> NZP_WE = 1;
    CPU -> rsMux_CTL = 2;
    CPU -> rtMux_CTL = 0;

    switch(sub_opcode) {
        case 0 : // binary 00
            value = (CPU -> R)[Rs] - (CPU -> R)[Rt];
            signed_value = unsigned_to_signed_short(value);
			SetNZP(CPU, signed_value);
            break;
        case 1 : // binary 01
			SetNZP(CPU, (CPU -> R)[Rs] - (CPU -> R)[Rt]);
            break;
        case 2 : // binary 10
            imm_seven = INSN_IMM7(instr);
            value = (CPU -> R)[Rs] - sext(imm_seven, 7);
            signed_value = unsigned_to_signed_short(value); 
			SetNZP(CPU, signed_value);
            break;
        case 3 : // binary 11
            u_imm_seven = INSN_IMM7(instr);
			SetNZP(CPU, (CPU -> R)[Rs] - u_imm_seven);
            break;
        default:
            printf("Error while parsing comparative instruction.\n");
    }
    WriteOut(CPU, output);
    CPU -> PC = CPU -> PC + 1;
}

/*
 * Parses rest of logical operation and prints out.
 */
void LogicalOp(MachineState* CPU, FILE* output)
{
	printf("entered LogicalOp.\n");

    unsigned short int Rd = INSN_11_9(instr);
    unsigned short int Rs = INSN_8_6(instr);
    unsigned short int Rt = INSN_2_0(instr);
    unsigned short int sub_opcode = INSN_5_3(instr);
    signed short int imm_five;
    unsigned short int Rd_value;
    

    ClearSignals(CPU);
    CPU -> regFile_WE = 1;
    CPU -> NZP_WE = 1;
    CPU -> rsMux_CTL = 0;
    CPU -> rtMux_CTL = 0;
    CPU -> rdMux_CTL = 0;
    global_Rd = Rd;

    if (sub_opcode == 0) { // binary 000 (AND)
        Rd_value = (CPU -> R)[Rs] & (CPU -> R)[Rt];
    } else if (sub_opcode == 1) { // binary 001 (NOT)
        Rd_value = ~(CPU -> R)[Rs];
    } else if (sub_opcode == 2) { // binary 010 (OR)
		// Rd = Rs - Rt
        Rd_value = (CPU -> R)[Rs] | (CPU -> R)[Rt];
    } else if (sub_opcode == 3) { // binary 011 (XOR)
		// Rd = Rs / Rt
        Rd_value = (CPU -> R)[Rs] ^ (CPU -> R)[Rt];
    } else if (sub_opcode >= 4) { // binary 1XX (AND IMM)
        imm_five = INSN_IMM5(instr);
		Rd_value = (CPU -> R)[Rs] & sext(imm_five, 5);
    } else {
        printf("Error while parsing logical instruction\n");
    }
	(CPU -> R)[Rd] = Rd_value;
	CPU -> regInputVal = Rd_value;
    SetNZP(CPU, Rd_value);
    
    WriteOut(CPU, output);
    CPU -> PC = CPU -> PC + 1;

}

/*
 * Parses rest of jump operation and prints out.
 */
void JumpOp(MachineState* CPU, FILE* output)
{
	printf("entered JumpOp.\n");

    unsigned short int Rs = INSN_8_6(instr);
    signed short int imm_eleven = INSN_11_0(instr);
    unsigned short int sub_opcode = INSN_11(instr);

    ClearSignals(CPU);

    WriteOut(CPU, output);

    switch(sub_opcode) {
        case 0 : // binary 0
            CPU -> PC = (CPU -> memory)[Rs];
            break;
        case 1 : // binary 1
            CPU -> PC = CPU -> PC + 1 + sext(imm_eleven, 11);
            break;
        default:
            printf("Error while parsing jump instruction.\n");
    }
    
}

/*
 * Parses rest of JSR operation and prints out.
 */
void JSROp(MachineState* CPU, FILE* output)
{
	printf("entered JSROp.\n");
    unsigned short int Rs = INSN_8_6(instr);
    signed short int imm_eleven = INSN_11_0(instr);
    unsigned short int sub_opcode = INSN_11(instr);
    unsigned short int Rd_value;

    ClearSignals(CPU);
    CPU -> regFile_WE = 1;
    CPU -> rdMux_CTL = 1;
    CPU -> NZP_WE = 1;
    Rd_value = CPU -> PC + 1;
    CPU -> regInputVal = Rd_value;
    SetNZP(CPU, Rd_value);
    global_Rd = 7;
    WriteOut(CPU, output);

    switch(sub_opcode) {
        case 0 : // binary 0
            (CPU -> memory)[7] = CPU -> PC + 1;
            CPU -> PC = (CPU -> memory)[Rs];
            break;
        case 1 : // binary 1
            (CPU -> memory)[7] = CPU -> PC + 1;
            CPU -> PC = (CPU -> PC & 0x8000) | (sext(imm_eleven, 11) << 4);
            break;
        default:
            printf("Error while parsing JSR instruction.\n");
    }
    
}

/*
 * Parses rest of shift/mod operations and prints out.
 */
void ShiftModOp(MachineState* CPU, FILE* output)
{
	printf("entered ShiftModOp.\n");
    unsigned short int Rd = INSN_11_9(instr);
    unsigned short int Rs = INSN_8_6(instr);
    unsigned short int Rt = INSN_2_0(instr);
    unsigned short int u_imm_four = INSN_4_0(instr);
    unsigned short int sub_opcode = INSN_5_4(instr);
    unsigned short int Rd_value;

    ClearSignals(CPU);
    CPU -> regFile_WE = 1;
    CPU -> NZP_WE = 1;
    CPU -> rsMux_CTL = 0;
    CPU -> rtMux_CTL = 0;
    CPU -> rdMux_CTL = 0;
    global_Rd = Rd;

    switch(sub_opcode) {
        case 0 : // binary 00
            Rd_value = (CPU -> R)[Rs] << u_imm_four;
            break;
        case 1 : // binary 01
            Rd_value = (signed short int) (CPU -> R)[Rs] >> u_imm_four;
            break;
        case 2 : // binary 10
            Rd_value = (CPU -> R)[Rs] >> u_imm_four;
            break;
        case 3 : // binary 11
            Rd_value = (CPU -> R)[Rs] % (CPU -> R)[Rt];
            break;
        default:
            printf("Error while parsing shift instruction.\n");
    }
    (CPU -> R)[Rd] = Rd_value;
	CPU -> regInputVal = Rd_value;
    SetNZP(CPU, Rd_value);
    WriteOut(CPU, output);
    (CPU -> PC)++;

}

/*
 * Parses rest of STR operations and prints out.
 */
void STROp(MachineState* CPU, FILE* output)
{
    printf("entered STROp.\n");

	unsigned short int Rs = INSN_8_6(instr);
    unsigned short int Rt = INSN_11_9(instr);
	signed short int imm_six = INSN_IMM6(instr);
    unsigned short int dmemAddr_temp;

	ClearSignals(CPU);
	CPU -> rsMux_CTL = 0;
    CPU -> rtMux_CTL = 1;
	CPU -> DATA_WE = 1;

    dmemAddr_temp = (CPU -> R)[Rs] + sext(imm_six, 6);

    // checking if we are attempting to write a code section address
    if (dmemAddr_temp < 0x1FFF || (dmemAddr_temp >= 0x8000 && dmemAddr_temp <= 0x9FFF)) {
        printf("error: illegal operation, code section address cannot be accessed.\n");
        error = 1;
        return;
    }

    // checking if we are attempting access an address in the OS section of memory when the processor is in user mode
    if (dmemAddr_temp >= 0xA000 && dmemAddr_temp <= 0xFFFF && INSN_PRIV_BIT(CPU -> PSR) == 0) {
        printf("error: illegal operation, OS section address cannot be accessed when processer is in user mode.\n");
        error = 1;
        return;
    }

    CPU -> dmemAddr = dmemAddr_temp;
    CPU -> dmemValue = (CPU -> R)[Rt];

	(CPU -> memory)[CPU -> dmemAddr] = CPU -> dmemValue;
    
    
    WriteOut(CPU, output);
    CPU -> PC = CPU -> PC + 1;
	
}

/*
 * Parses rest of LDR operation and prints out.
 */
void LDROp(MachineState* CPU, FILE* output)
{
    printf("entered LDROp.\n");

    unsigned short int Rd = INSN_11_9(instr);
    unsigned short int Rs = INSN_8_6(instr);
	unsigned short int imm_six = INSN_IMM6(instr);
    unsigned short int Rd_value;
    unsigned short int dmemAddr_temp;

    ClearSignals(CPU);
	CPU -> rsMux_CTL = 0;
    CPU -> rtMux_CTL = 0;
    CPU -> regFile_WE = 1;
    CPU -> NZP_WE = 1;
    global_Rd = Rd;

    dmemAddr_temp = (CPU -> R)[Rs] + sext(imm_six, 6);

    // checking if we are attempting to read a code section address
    if (dmemAddr_temp < 0x1FFF || (dmemAddr_temp >= 0x8000 && dmemAddr_temp <= 0x9FFF)) {
        printf("error: illegal operation, code section address cannot be accessed.\n");
        error = 1;
        return;
    }

        // checking if we are attempting access an address in the OS section of memory when the processor is in user mode
    if (dmemAddr_temp >= 0xA000 && dmemAddr_temp <= 0xFFFF && INSN_PRIV_BIT(CPU -> PSR) == 0) {
        printf("error: illegal operation, OS section address cannot be accessed when processer is in user mode.\n");
        error = 1;
        return;
    }

    
    CPU -> dmemAddr = dmemAddr_temp;
    Rd_value = (CPU -> memory)[CPU -> dmemAddr];
    CPU -> dmemValue = Rd_value;
    
    (CPU -> R)[Rd] = Rd_value;
    CPU -> regInputVal = Rd_value;

    SetNZP(CPU, Rd_value);
    WriteOut(CPU, output);
    CPU -> PC = CPU -> PC + 1;
}

/*
 * Parses rest of RTI operation and prints out.
 */
void RTIOp(MachineState* CPU, FILE* output) 
{
    printf("entered RTIOp.\n");

    ClearSignals(CPU);

    // PSR[15] = 0
    CPU -> PSR = CPU -> PSR & 0x7FFF;
    WriteOut(CPU, output);

    CPU -> PC = (CPU -> R)[7];
    
}

/*
 * Parses rest of constant operation and prints out.
 */
void ConstOp(MachineState* CPU, FILE* output)
{
    printf("entered ConstOp.\n");
    unsigned short int Rd_value;
    unsigned short int Rd = INSN_11_9(instr);
    signed short int imm_nine = INSN_IMM9(instr);
    
    ClearSignals(CPU);
	CPU -> rsMux_CTL = 0;
    CPU -> rtMux_CTL = 0;
    CPU -> regFile_WE = 1;
    CPU -> NZP_WE = 1;
    global_Rd = Rd;

    Rd_value = sext(imm_nine, 9);
    (CPU -> R)[Rd] = Rd_value;
    CPU -> regInputVal = Rd_value;
    SetNZP(CPU, Rd_value);
    WriteOut(CPU, output);
    CPU -> PC = CPU -> PC + 1;

}
void HiConstOp(MachineState* CPU, FILE* output)
{
    printf("entered HiConstOp.\n");

    unsigned short int Rd = INSN_11_9(instr);
    unsigned short int u_imm_eight = INSN_IMM8(instr);
    unsigned short int Rd_value;
    
    ClearSignals(CPU);
	CPU -> rsMux_CTL = 2;
    CPU -> rtMux_CTL = 0;
    CPU -> rdMux_CTL = 0;
    CPU -> regFile_WE = 1;
    CPU -> NZP_WE = 1;
    global_Rd = Rd;

    Rd_value = ((CPU -> R)[Rd] & 0xFF) | (u_imm_eight << 8);

    (CPU -> R)[Rd] = Rd_value;
    CPU -> regInputVal = Rd_value;
    SetNZP(CPU, Rd_value);
    WriteOut(CPU, output);
    CPU -> PC = CPU -> PC + 1;

}

/*
 * Parses rest of trap operation and prints out.
 */
void TrapOp(MachineState* CPU, FILE* output)
{
    printf("entered TrapOp.\n");

    unsigned short int u_imm_eight = INSN_IMM8(instr);
    unsigned short int Rd_value;

    ClearSignals(CPU);
    CPU -> rdMux_CTL = 1;
    CPU -> regFile_WE = 1;
    CPU -> NZP_WE = 1;
    global_Rd = 7;

    Rd_value = CPU -> PC + 1;
    CPU -> regInputVal = Rd_value;
    (CPU -> R)[7] = Rd_value;
    CPU -> PSR = CPU -> PSR | 0x8000;

    SetNZP(CPU, Rd_value);
    WriteOut(CPU, output);

    CPU -> PC = (0x8000 | u_imm_eight); 

}


/*
 * Set the NZP bits in the PSR.
 */
void SetNZP(MachineState* CPU, short result)
{

    // reset PSR bits[0:2] all back to 0
    CPU -> PSR = CPU -> PSR & 0xFFFE; 
    CPU -> PSR = CPU -> PSR & 0xFFFD; 
    CPU -> PSR = CPU -> PSR & 0xFFFB; 

    if (result > 0) {
        // set PSR bit[0] = 1, bit[1] = 0, bit[2] = 0
        CPU -> PSR = CPU -> PSR | 0x0001;
        CPU -> NZPVal = 1;
    } else if (result == 0) {
        // set PSR bit[0] = 0, bit[1] = 1, bit[2] = 0
        CPU -> PSR = CPU -> PSR | 0x0002;
        CPU -> NZPVal = 2;
    } else {
        // set PSR bit[0] = 0, bit[1] = 0, bit[2] = 1
        CPU -> PSR = CPU -> PSR | 0x0004;
        CPU -> NZPVal = 4;
    }
    
}

/*
 * Prints an unsigned short int to a file in its binary form
 */
int fprintf_unsigned_to_binary(unsigned short int num, FILE* output) 
{

	int i;

	if (output == NULL) {
		printf("error in fprintf_unsigned_to_binary");
		return -1;
	}

    for (i = 15; i >= 0; i--) {
        if (num & (1 << i)) {
            fputc('1', output);
        } else {
            fputc('0', output);
        }
    }

	return 1;

}


/*
 * Converts the value of a 2C unsigned short int to its signed short int value
 */
signed short int unsigned_to_signed_short(unsigned short int num) 
{
    if (num & (1 << 15)) {
        return (signed short int) (num - (1 << 16));
    } else {
        return (signed short int) num;
    }
}

/*
 * Sign extends an unsigned short int
 */
signed short int sext(signed short int num, int num_length)
{
    int bit_of_interest = num_length - 1;

    if (((num >> bit_of_interest) & 0x0001) == 0) {
        return num;
    } else {
        for (int i = bit_of_interest + 1; i < 16; i++) {
            num = num | (1 << i);
        }
    }

    return num;

}
