/*
 * trace.c: location of main() to start the simulator
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "loader.h"

// Global variable defining the current state of the machine
MachineState* CPU;

int main(int argc, char** argv)
{

    int i = 0;
    MachineState myCPU;
	char* n;
	char* filename_one = NULL;
	char* filename_two = NULL;	
	char* filename_three = NULL;		
	char* output_filename = NULL;
	FILE* output_file;
	
	CPU = &myCPU;

	// initialize the state of the simulator just the way PennSim does it after a reset operation
	Reset(CPU);

	if (argc < 3 || argc > 5) {
		printf("error1: incorrect arguments\n");
		return -1;
	}

	output_filename = argv[1];
	// check that output file argument is a .txt file
	n = strstr(output_filename, ".txt");
	if (n == NULL) {
		printf("error1: incorrect arguments\n");
		return -1;
	}

	if (argc >= 3) {
		filename_one = argv[2];

		n = strstr(filename_one, ".obj");

		// check that input file exists and is a .obj file
		if (n != NULL && fopen(filename_one, "r") != NULL) {
			ReadObjectFile(filename_one, CPU);
		} else {
			printf("error1: incorrect arguments\n");
			return -1;
		}

	}

	if (argc >= 4) {
		filename_two = argv[3];

		n = strstr(filename_one, ".obj");

		// check that input file exists and is a .obj file
		if (n != NULL && fopen(filename_two, "r") != NULL) {
			ReadObjectFile(filename_two, CPU);
		} else {
			printf("error1: incorrect arguments\n");
			return -1;
		}
	}

	if (argc == 5) {
		filename_three = argv[4];
			
		n = strstr(filename_one, ".obj");

		// check that input file exists and is a .obj file
		if (n != NULL && fopen(filename_three, "r") != NULL) {
			ReadObjectFile(filename_three, CPU);
		} else {
			printf("error1: incorrect arguments\n");
			return -1;
		}
	}

	output_file = fopen(output_filename, "w");
	
	while (CPU -> PC != 0x80FF) {
		if (UpdateMachineState(CPU, output_file) == -1) {
			break;
		}
	}
	
	fclose(output_file);

    return 0;
}

