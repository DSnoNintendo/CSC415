/**************************************************************
* Class::  CSC-415-01 Fall 2024
* Name:: Daryl Stronge
* Student ID:: 917844673
* GitHub-Name:: DSNoNintendo
* Project:: Assignment 3 – Simple Shell
*
* File:: Stronge_Daryl_HW3_main.h
*
* Description:: Simple shell to emulate a Linux command line at
                a simple scale, including pipe support
*
**************************************************************/

#define LINE_LENGTH 159
#define TEMP_FILE_NAME "temp"
#define MAX_ARGUMENTS 10
#define MAX_PIPES 20

void redirect_std(int pipe[], int end, int std);

void run_commands(char **cmd_arrays[], int num_cmd_arrays, int num_pipes);

void add_cmd_args_to_cmd_arrays(char **cmd_arrays[], 
                                char *cmd_args[], 
                                int cmd_arrays_idx, 
                                int arg_idx);

void free_cmd_arrays(char **cmd_arrays[], int num_cmd_arrays);

int main(int argc, char *argv[]);