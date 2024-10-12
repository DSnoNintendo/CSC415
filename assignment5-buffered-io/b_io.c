/**************************************************************
 * Class::  CSC-415-01 Spring 2024
 * Name:: Daryl Stronge
 * Student ID:: 917844673
 * GitHub-Name:: DSNoNintendo
 * Project:: Assignment 5 – Buffered I/O read
 *
 * File:: b_io.c
 *
 * Description:: Using a file control block, read data from 
 * 					memory blocks and copy the data to a user
 * 					buffer.
 *
 **************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "b_io.h"

#define MAXFCBS 20 // The maximum number of files open at one time


// static array of file control blocks
b_fcb fcbArray[MAXFCBS];

// Indicates that the file control block array has not been initialized
int startup = 0;

// Method to initialize our file system / file control blocks
// Anything else that needs one time initialization can go in this routine
void b_init()
{
	if (startup)
		return; // already initialized

	// init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
	{
		fcbArray[i].fi = NULL; // indicates a free fcbArray
	}

	startup = 1;
}

// Method to get a free File Control Block FCB element
b_io_fd b_getFCB()
{
	for (int i = 0; i < MAXFCBS; i++)
	{
		if (fcbArray[i].fi == NULL)
		{
			fcbArray[i].fi = (fileInfo *)-2; // used but not assigned
			return i;						 // Not thread safe but okay for this project
		}
	}

	return (-1); // all in use
}

// b_open is called by the "user application" to open a file.  This routine is
// similar to the Linux open function.
// You will create your own file descriptor which is just an integer index into an
// array of file control blocks (fcbArray) that you maintain for each open file.
// For this assignment the flags will be read only and can be ignored.

b_io_fd b_open(char *filename, int flags)
{
	if (startup == 0)
		b_init(); // Initialize our system

	fileInfo *info = GetFileInfo(filename);

	if (info != NULL)
	{
		printf("File [%s] found.\n", filename);
		b_fcb fcb =
			{
				.fi = info,
				.buffer = malloc(B_CHUNK_SIZE),
				.curr_block = info->location,
				.curr_byte = B_CHUNK_SIZE,
				.remaining_bytes = 0,
				.bytes_copied = 0,
			};
		b_io_fd fileDescriptor = b_getFCB();
		fcb.fd = fileDescriptor;
		fcbArray[fileDescriptor] = fcb;

		return fileDescriptor;
	}
	else
	{
		printf("File [%s] not found.\n", filename);
		return -1;
	}
}

void reset_fcb(b_fcb *fcb)
{
	fcb->remaining_bytes = B_CHUNK_SIZE;
	fcb->curr_byte = 0;
	memset(fcb->buffer, 0, B_CHUNK_SIZE);
}

int copy_to_caller_buffer(char *dest, char *source, int count, b_fcb *fcb)
{
	memcpy(dest, source, count);
	fcb->curr_byte += count;
	fcb->bytes_copied += count;
	fcb->remaining_bytes -= count;
	return count;
}

int all_bytes_read(b_fcb* fcb) {
	// if all bytes from file have been copied, return true
	if (fcb->bytes_copied >= fcb->fi->fileSize)
	{
		return 1;
	}
	return 0;
}

int b_read(b_io_fd fd, char *buffer, int count)
{
	if (startup == 0)
		b_init();

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1);
	}

	if (fcbArray[fd].fi == NULL)
	{
		return -1;
	}

	b_fcb *fcb = &fcbArray[fd];
	if (all_bytes_read(fcb)) return 0;

	int bytes_copied; // bytes copied to caller buffer in a single `copy_to_caller_buffer` call
	int total_bytes_copied = 0; // total bytes copied to caller buffer in this function call
	
	/*
		If all bytes have been copied from File Control Block buffer or program 
		is initializing, read bytes from memory block to File Control Block buffer.
	*/
	if (fcb->remaining_bytes == 0) {
		reset_fcb(fcb);
		LBAread(fcb->buffer, 1, fcb->curr_block);
	} 

	// If all requested bytes have been copied to the caller buffer, end loop
	while (total_bytes_copied < count)
	{
			// If file control block buffer has more uncopied bytes than requested
			if (fcb->remaining_bytes > count)
			{
				/*
					If the amount of uncopied bytes from the file is less than the 
					requested amount, only copy the uncopied bytes.
					If not, copy the requested amount. There is no need to continue 
					the loop if this logic runs.
				*/ 
				bytes_copied = copy_to_caller_buffer(buffer + total_bytes_copied,
													fcb->buffer + fcb->curr_byte,
													(fcb->fi->fileSize - fcb->bytes_copied < count) 
														? fcb->fi->fileSize - fcb->bytes_copied :
														count,
													fcb);
				total_bytes_copied += bytes_copied;

				return total_bytes_copied;
			}
			else
			{
				/*
					If the requested number of bytes is less than file control block
					has available, copy the remaining bytes.
				*/
				bytes_copied = copy_to_caller_buffer(buffer + total_bytes_copied, 
													fcb->buffer + fcb->curr_byte, 
													fcb->remaining_bytes, 
													fcb);
				count -= bytes_copied;
				total_bytes_copied += bytes_copied;

				/*
					After reading the remaining bytes from the file control block 
					buffer, if the requested number of bytes exceeds the size of a 
					full block, an entire block can be read to the buffer.
				*/
				if (count > B_CHUNK_SIZE) {
					fcb->curr_block++;
					LBAread(buffer+total_bytes_copied, 1, fcb->curr_block);
					fcb->bytes_copied += B_CHUNK_SIZE;
					total_bytes_copied += B_CHUNK_SIZE;
					count -= B_CHUNK_SIZE;
				}

				reset_fcb(fcb);
				fcb->curr_block++;
				LBAread(fcb->buffer, 1, fcb->curr_block);
			}
		
	}

	return total_bytes_copied;
}

// b_close frees and allocated memory and places the file control block back
// into the unused pool of file control blocks.
int b_close(b_io_fd fd)
{
	b_fcb *fcb = &fcbArray[fd];
	free(fcb->buffer);
	fcb->buffer = NULL;	
}
	
