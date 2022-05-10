/*********************************************************************/
/*                                                                   */
/*          Module name: execution_module.h                          */
/*          Description:                                             */
/*              This module provides functions to execute programs.  */
/*                                                                   */
/*********************************************************************/

#ifndef EXECUTION_MODULE_H
#define EXECUTION_MODULE_H

/* directives */
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "./string_module.h"

/* macros */
#ifndef FAILURE
    #define FAILURE 0
#endif
#ifndef SUCCESS
    #define SUCCESS 1
#endif

#define READ_END 0
#define WRITE_END 1

/* globals */
extern char* cmds[]; 
extern int n_cmds;

/* static function prototypes */
static int     generate_process( int fd_in, int fd_out, char* prog[] );
static void    execute_and_pipe( int n_pipes, int pipe_loc[], int fd_in, int fd_out );

/* standard program execution */
void    execute( int infile_pos, int outfile_pos, int n_pipes, int pipe_loc[] );

#endif
