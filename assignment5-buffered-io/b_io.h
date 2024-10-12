/**************************************************************
* Class::  CSC-415-0A Spring 2024
* Name::   Professor Bierman
* Student ID:: N/A
* GitHub-Name:: N/A
* Project:: Assignment 5 – Buffered I/O read
*
* File:: b_io.h
*
* Description:: Definitino of b_io_fd type and the prototypes
*               of the functions accessible by user applications
*
**************************************************************/

#ifndef _B_IO_H
#define _B_IO_H
#include "fsLowSmall.h"

typedef int b_io_fd;

// This structure is all the information needed to maintain an open file
// It contains a pointer to a fileInfo strucutre and any other information
// that you need to maintain your open file.
typedef struct b_fcb
{
	fileInfo *fi; // holds the low level systems file info
	char *buffer;
	int remaining_bytes; // uncopied bytes from last `LBAread` call
	int bytes_copied; // total bytes copied from buffer
	int curr_block; // tracks position of current memory block
	int curr_byte; // tracks position of last byte copied
	int buffer_read_offset; // where to continue in from buffer furing block reads
	int fd; // file descriptor
} b_fcb;


b_io_fd b_open (char * filename, int flags);
int b_read (b_io_fd fd, char * buffer, int count);
int b_close (b_io_fd fd);

int all_bytes_read(b_fcb* fcb);
int copy_to_user_buffer(char *dest, char *source, int count, b_fcb *fcb);
void reset_fcb(b_fcb *fcb);

#endif

