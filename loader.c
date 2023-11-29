/*
 * loader.c : Defines loader functions for opening and loading object files
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "loader.h"

// memory array location
unsigned short memoryAddress;

// helper function which swaps the position of lower 8 bits with upper 8 bits, swapping the endianess
unsigned short int swap_endianess(unsigned short int input) {

    unsigned short int ret; 

    ret = (input >>8) | (input <<8);

    return ret;
}

/*
 * Read an object file and modify the machine state as described in the writeup
 */
int ReadObjectFile(char* filename, MachineState* CPU)
{
  
    unsigned short int h;
    unsigned short int addr;
    unsigned short int num;
    unsigned short int x;
    MachineState temp;
    int i;

    FILE* my_file;

    my_file = fopen(filename, "r");

    // read in two bytes
	while (fread(&h, 1, sizeof(unsigned short int), my_file) == sizeof(unsigned short int)) {

		h = swap_endianess(h);

		if (h == 0xCADE) {

			fread(&addr, 1, sizeof(unsigned short int), my_file);
			addr = swap_endianess(addr);
			fread(&num, 1, sizeof(unsigned short int), my_file);
			num = swap_endianess(num);

			//printf("%d\n", num);
			
			for (i = 0; i < num; i++) {
				fread(&x, 1, sizeof(unsigned short int), my_file);
				x = swap_endianess(x);

				(CPU -> memory)[addr + i] = x;
			}
      
		} else if (h == 0xDADA) {

			fread(&addr, 1, sizeof(unsigned short int), my_file);
			addr = swap_endianess(addr);
			fread(&num, 1, sizeof(unsigned short int), my_file);
			num = swap_endianess(num);
			
			for (i = 0; i < num; i++) {
				fread(&x, 1, sizeof(unsigned short int), my_file);
				x = swap_endianess(x);

				(CPU -> memory)[addr + i] = x;
			}

		} else if (h == 0xC3B7) {
			
			// move file pointer to parse through this section without doing anything
			fread(&addr, 1, sizeof(unsigned short int), my_file);
			fread(&num, 1, sizeof(unsigned short int), my_file);
			for (i = 0; i < num; i++) {
				fread(&x, 1, sizeof(unsigned short int), my_file);
			}

		} else if (h == 0xF17E) {

			// move file pointer to parse through this section without doing anything
			fread(&num, 1, sizeof(unsigned short int), my_file);
			for (i = 0; i < num; i++) {
				fread(&x, 1, sizeof(unsigned short int), my_file);
			}

		} else if (h == 0x715E) {
			
			// move file pointer to parse through this section without doing anything
			fread(&addr, 1, sizeof(unsigned short int), my_file);
			fread(&num, 1, sizeof(unsigned short int), my_file);
			fread(&x, 1, sizeof(unsigned short int), my_file);

		} else {
			printf("error2: ReadObjectFile failed, file may not be formatted properly\n");
			return -1;
		}

	}

    return 0;

}