# LC4 CPU Simulator

A command-line version of the assembly-level LC4 CPU simulator that is part of PennSim. This involves loading and processing machine code files (binary files produced by the LC4 assembler) and executing them by keeping track of 
internal state (PC, PSR, registers, control signals, etc.). A trace text file is generated as an output that contains information from each LC4 “cycle” as the program executes the loaded machine code.
