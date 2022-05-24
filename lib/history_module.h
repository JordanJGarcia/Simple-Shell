/*********************************************************************/
/*                                                                   */
/*          Module name: history_module.h                            */
/*          Description:                                             */
/*              This module provides structures and functions to     */
/*              store commands so we can keep track of what user     */
/*              enters.                                              */
/*                                                                   */
/*********************************************************************/

#ifndef HISTORY_MODULE_H 
#define HISTORY_MODULE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "string_module.h"

/* macros */
#define CMD_LIMIT 50
#define HISTORY_LIMIT 50

#ifndef FAILURE
    #define FAILURE 0
#endif
#ifndef SUCCESS
    #define SUCCESS 1
#endif

/* globals */
//char* history[HISTORY_LIMIT];

/* function prototypes */
int     add_to_history( char* );
void    print_history( FILE* );
int     write_history_to_file( void );
int     free_history( void );

#endif
