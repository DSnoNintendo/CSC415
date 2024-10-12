/**************************************************************
* Class::  CSC-415-01 Fall 2024
* Name:: Daryl Stronge
* Student ID:: 917844673
* GitHub-Name:: DSNoNintendo
* Project:: Assignment 3 – Simple Shell
*
* File:: Stronge_Daryl_HW3_main.c
*
* Description:: Simple shell to emulate a Linux command line at
                a simple scale, including pipe support
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <Stronge_Daryl_HW3_main.h>

void redirect_std(int pipe[], int end, int std)
{
    if (dup2(pipe[end], std) == -1)
    { 
        printf("dup2 failed");
        exit(1);
    }
}

void run_commands(char **cmd_arrays[], int num_cmd_arrays, int num_pipes)
{
    pid_t pid;
    int status; // thread status

    /*
    Allows command output to be written from pipe-to-pipe in instances of multiple pipes.
    (logic explained below)
    */
    int file_descriptors[num_pipes][2]; 
    for (int i = 0; i < num_pipes; i++)
    {
        if (pipe(file_descriptors[i]) == -1)
        {
            printf("pipe failed");
            exit(EXIT_FAILURE);
        }
    }

    /*
    Pipe Example: cat Makefile | wc | wc -l
                               ^    ^

    On the first iteration of the following for-loop, STDOUT is redirected to fd[0][write], and 
    STDIN is redirected to fd[0][read]. fd[0] represents the pipe between `cat Makefile` and `wc`.
    This will allow the command in the next iteration of the for-loop to read the output of 
    the `execvp`.

    On the second iteration, since there is another command after `wc`, STDOUT is 
    redirected to fd[1][write] and STDIN is redirected to fd[1][read]. fd[1] represents the pipe
    between `wc` and `wc -l`. `execvp()` will execute using the output from the last iteration.

    On the final iteration, the pipe logic will be skipped because it not needed. The final
    command will be executed using the output from the last iteration.
    */ 
    for (int i = 0; i < num_cmd_arrays; i++)
    {
        char **cmd = cmd_arrays[i];
        if (i + 1 < num_cmd_arrays) // if i + 1 < num_cmd_arrays, next command exists
        {
            pid = fork();

            if (pid == -1)
            {
                perror("fork failed");
                exit(EXIT_FAILURE);
            }

            if (pid == 0)
            {
                // close read end of pipe
                close(file_descriptors[i][0]);
                redirect_std(file_descriptors[i], 1, STDOUT_FILENO);
                execvp(cmd[0], cmd);
                exit(1); // exit one 
            }
            else
            {
                // Close the write end of the pipe
                close(file_descriptors[i][1]);
                // redirect STDIN to read end of pipe
                redirect_std(file_descriptors[i], 0, STDIN_FILENO);
                close(file_descriptors[i][0]);
                waitpid(pid, &status, 0);
            }
        }
        else
        {
            pid = fork();
            if (pid == -1)
            {
                perror("fork failed");
                exit(1);
            }
            if (pid == 0)
            {
                execvp(cmd[0], cmd);
                exit(1);
            }
            else
            {
                waitpid(pid, &status, 0);
                printf("PID: %d\nStatus: %d\n", pid, WEXITSTATUS(status));
            }
        }
    }
}

void add_cmd_args_to_cmd_arrays(char **cmd_arrays[], 
                                char *cmd_args[], 
                                int cmd_arrays_idx, 
                                int arg_idx)
{
    /*
    In order to store command argument arrays to the two-dimensional array, the arguments must 
    be deep-copied because they are character pointers.

    Memory is allocated this way because it ensures only the required amount of memory is 
    allocated.
    */

    // arg_idx + 1 = number of character pointers in given `cmd_args[]`
    cmd_arrays[cmd_arrays_idx] = (char **)malloc((arg_idx + 1) * sizeof(char *));
    if (cmd_arrays[cmd_arrays_idx] == NULL) {
        printf("malloc failed");
        exit(1);
    }
    for (int i = 0; i < arg_idx + 1; i++)
    { 
        if (cmd_args[i] != NULL)
        {
            cmd_arrays[cmd_arrays_idx][i] = (char *)malloc(strlen(cmd_args[i] + 1));
            if (cmd_arrays[cmd_arrays_idx][i] == NULL)
            {
                printf("malloc failed");
                exit(1);
            }

            strcpy(cmd_arrays[cmd_arrays_idx][i], cmd_args[i]);
        }
        else
        {
            /*
            The last index of a command array must be set to NULL in order to pass it to an
            `execvp` call
            */
            cmd_arrays[cmd_arrays_idx][i] = NULL;
            return;
        }
    }
}

void free_cmd_arrays(char **cmd_arrays[], int num_cmd_arrays)
{
    // memory allocated by `add_cmd_args_to_cmd_arrays` must be freed
    for (int i = 0; i <= num_cmd_arrays; i++)
    {
        if (cmd_arrays[i] != NULL)
        {
            int j = 0;
            /*
            loop and free until `NULL` because NULL was set to the last value
            */ 
            while (cmd_arrays[i][j] != NULL) 
            {
                free(cmd_arrays[i][j]); // Free each allocated string
                cmd_arrays[i][j] = NULL;
                j++;
            }
            
        }
        free(cmd_arrays[i]);
        cmd_arrays[i] = NULL;
        
    }
}

int main(int argc, char *argv[])
{   
    char* prompt;
    if (argc > 1) {
        prompt = argv[1];
    }
    else {
        prompt = "> ";
    }

    while (1)
    {
        char cmd_line[LINE_LENGTH] = ""; // stores user input

        char **cmd_arrays[MAX_PIPES] = {NULL}; // stores each command in a 2D Array
        char *cmd_args[MAX_ARGUMENTS] = {NULL}; // stores each command argument
        
        int num_cmd_arrays = 0; // stores index of cmd_arrays
        int cmd_args_idx = 0; // stores index of `cmd_args`

        int num_pipes = 0; // stores number of pipes in user input for `run_commands` call

        
        printf("%s", prompt);
        fgets(cmd_line, LINE_LENGTH, stdin);
        printf("\n");

        // `\n` is included in this `strcmp` call because user input will include a return key.
        if (strcmp(cmd_line, "exit\n") == 0) break;

        char *cmd_token = strtok(cmd_line, " \n");
        if (cmd_token == NULL)
        {
            /*
                Addresses potential infite loop issues after calling run_commands.
                In certain issues `fgets` forgoes user input and `cmd_token` is set to NULL 
                after a call to `run_commands`.
                setting `cmd_token` to "\0" will be addressed in the follwing while-loop.
            */
            cmd_token = "\0"; 
        }

        while (cmd_token != NULL)
        {
            if (strcmp(cmd_token, "\0") == 0) // Handles EOF
            {
                return 0;
            }
            else if (strcmp(cmd_token, "|") == 0)
            {
                // If a pipe is detected, first command has, been entered; append to `cmd_arrays`
                add_cmd_args_to_cmd_arrays(cmd_arrays, cmd_args, num_cmd_arrays, cmd_args_idx);
                num_cmd_arrays++;
                num_pipes++;

                // after pipe is handled, clear cmd_args for next command
                memset(cmd_args, 0, sizeof(cmd_args));
                cmd_args_idx = 0;
            }
            else
            {
                cmd_args[cmd_args_idx] = cmd_token;
                cmd_args_idx++;
            }

            cmd_token = strtok(NULL, " \n");
        }

        // After `strtok` loop ends, remaining `cmd_args` must be added to `cmd_arrays`
        // if cmd_args_idx is > 0, than a populated `cmd_args` array exists.
        if (cmd_args_idx)
        {
            add_cmd_args_to_cmd_arrays(cmd_arrays, cmd_args, num_cmd_arrays, cmd_args_idx);
            num_cmd_arrays++;
        }

        run_commands(cmd_arrays, num_cmd_arrays, num_pipes);
        free_cmd_arrays(cmd_arrays, num_cmd_arrays);
    }
}
