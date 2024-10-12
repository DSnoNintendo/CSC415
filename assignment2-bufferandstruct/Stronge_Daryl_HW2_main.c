/**************************************************************
* Class::  CSC-415-01 Fall 2024
* Name:: Daryl Stronge
* Student ID:: 917844673
* GitHub-Name:: DSnoNintendo
* Project:: Assignment 2 – Struct & Buffer
*
* File:: Stronge_Daryl_HW2_main.c
*
* Description:: Instantiate a struct using malloc and write
                it to memory
                Write an algorithm to copy strings of unknown
                sizes to fix-sized buffers, and write the
                buffers to memory. 
*
**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assignment2.h>

int main(int argc, char *argv[])
{
    char *firstName = argv[1];
    char *lastName = argv[2];
    char *message = argv[3];

    /*
        The personalInfo struct was instantiated as a pointer because 
        because pi_ptr is pointing to the memory created by the malloc
        call.
    */
    struct personalInfo *pi_ptr = malloc(
        sizeof(personalInfo)
    );

    /*
        Memory must be allocated to store the `firstName` and `lastName` 
        arguments to their respective `personalInfo` member variables.
        In this case, the `firstName` and `lastName` arguments are 8 bytes each.

        Memory does not need to be allocated for `studentID`, `level`, or `languages`
        because they are integers. Memory is automatically allocated for integers
        and other primitive data types when a struct is instantiated.
    */
    pi_ptr->firstName = malloc(sizeof(firstName));
    strcpy(pi_ptr->firstName, firstName);

    pi_ptr->lastName = malloc(sizeof(lastName));
    strcpy(pi_ptr->lastName, lastName);

    pi_ptr->studentID = 917844673;
    pi_ptr->level = 20;
    pi_ptr->languages = (KNOWLEDGE_OF_CPLUSPLUS +
                            KNOWLEDGE_OF_MIPS_ASSEMBLER +
                            KNOWLEDGE_OF_JAVA
    );

    /*
        `strncpy` is used here instead of `strcpy` because it allows for the 
        specification of the maximum number of characters to be copied. 
        Because `pi_ptr->message` is a character array with a size of 100, 
        a maximum of 100 characters are copied from the message agument,
        regardless of how long it is.
    */
    strncpy(pi_ptr->message, message, 100);

    writePersonalInfo(pi_ptr);

    /*
        All previously malloc'd memory can be freed once the struct 
        is written to memory.
    */ 
    free(pi_ptr->firstName);
    free(pi_ptr->lastName);
    free(pi_ptr);

    /*
        Memory in the form of bytes equivalent to the number defined by
        `BLOCK_SIZE` is allocated for the buffer. This means that 
        the buffer will only be able to accept the amount of bytes 
        specified by `BLOCK_SIZE`.

        `buffer_pos` keeps track of the index of the next available
        byte in the buffer
    */ 
    char *buffer = malloc(BLOCK_SIZE);
    int buffer_pos = 0; 
    const char *str = getNext();
    int string_length;
    
    while (str != NULL)
    {
        string_length = strlen(str);

        /*
            This if-statement exists for cases in which a string
            returned from getNext() can be copied to a buffer as is,
            with no need for segmentation.

            if `string_length` is less-than-or-equal-to the difference
            of the `BLOCK_SIZE` and `buffer_pos`, the string can be
            copied to the buffer without the need for intervention.

            `buffer_pos` is used in `memcpy` calls as an index to keep track of 
            where the next available byte is for the next time memcpy is called.
        */
        if (string_length <= BLOCK_SIZE - buffer_pos)
        {
            memcpy(buffer + buffer_pos, str, string_length);
            buffer_pos += strlen(str);

            str = getNext();
        }
        /*
            This else-statement exists for cases in which a string returned
            from `getNext()` is too large to be copied to the buffer without 
            segmentation.
        */
        else
        {            
            /*
                In this specific `memcpy` call, the specified number of bytes 
                being copied is the difference of `BLOCK_SIZE` and `buffer_pos` (n).
                This difference will return the exact number of bytes needed to fill
                the buffer to capacity.

                Because only the first `n` bytes in the string will be copied to the 
                buffer, the rest of the string will need to be copied to a new buffer.
            */
            memcpy(buffer + buffer_pos, str, BLOCK_SIZE - buffer_pos);
            commitBlock(buffer);

            /*
                `memset` is used to clear the contents of `buffer` after a block is committed. 
                Reallocating memory using `malloc` would make the previously allocated memory 
                inaccessible, leading to a memory leak.
            */
            memset(buffer, 0, BLOCK_SIZE);

            /*
                After the filled buffer is committed and memory is allocated for a new one,
                the remaining bytes of the string can be copied to the new buffer.

                In this specific `memcpy` call, `buffer_pos` is not used in the first
                argument as it previously has been. This is because the remaining bytes
                of the string are being written to a new buffer at an "index" of 0.
                `buffer_pos` also has not been reset because its value is needed for 
                further calculations.

                In the second argument of this specific `memcpy` call, the start position 
                of the bytes being copied is set to where the string was segmented before 
                the previous buffer was committed.

                In the third argument of this specific `memcpy` call, the amount of bytes
                being copied is set to the amount of bytes that were excluded from the 
                previous buffer.
            */
            memcpy(buffer, 
                    str + (BLOCK_SIZE - buffer_pos), 
                    string_length - (BLOCK_SIZE - buffer_pos)
            );

            buffer_pos = string_length - (BLOCK_SIZE - buffer_pos);

            str = getNext();
        };

        /*
            This if-statement exists for an edge-case in which
            strings returned from getNext() fill a buffer to capacity 
            without the need for segmentation.

            If the length of the buffer string is equal to `BLOCK_SIZE`,
            the buffer has reached its capacity and a new block can be
            committed.
        */
        if (strlen(buffer) == BLOCK_SIZE)
        {
            commitBlock(buffer);
            buffer_pos = 0;
            memset(buffer, 0, BLOCK_SIZE);
        };
    };

    /*
        This if-statement exists for an edge-case in which
        the while-loop is complete, and there is a buffer that hasn't
        yet been commited to memory.
    */
    if (strlen(buffer) > 0)
    {
        commitBlock(buffer);
    };

    /*
        The memory allocated for `buffer` is freed here and not in the
        while-loop to avoid memory leaks.
    */
    free(buffer);

    return checkIt();
}
