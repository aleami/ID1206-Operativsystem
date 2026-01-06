# Question 8: Memory-mapped Files

## Overview
This program demonstrates file-backed memory mapping using `mmap()`.  
A parent and a child process communicate by reading from and writing to `file_to_map.txt`, which is memory mapped.

## Files
- assignment8.c – Source code.
- Makefile – Used for compiling the program.

## Compilation
To compile the program, run:
gcc assignment8.c -o q8

## Execution
Run the program with:
`./q8`
Note: The file `file_to_map.txt` must be created with a size of 1 MB before running the program.


## Authors
Group 37
Aleena Arsalan Amir
Araceli Prieto Gallego
