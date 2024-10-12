/**************************************************************
* Class::  CSC-415-0# Fall 2023
* Name:: Daryl Stronge
* Student ID:: 917844673
* GitHub-Name:: DSnoNintendo
* Project:: Assignment 1 – Command Line Arguments
*
* File:: stronge_daryl_HW1_main.c
*
* Description:: Print the number of arguments passed to the 
                program and its contents
**************************************************************/

#include <stdio.h>

int main(int argc, char *argv[]) {
    /**
    * Given an array of arguments and its length, 
    * print the length of the array and 
    * each of its arguments
    * 
    * @param int `argc` - the length of the given array
    * @param array `argv` - array
    */

    printf("There were %d arguments on the command line\n", argc);
 
    for (int i=0; i < argc; i++) {
        printf("Argument %d:    %s\n", i, argv[i]);
    }
}
