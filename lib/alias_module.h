/*********************************************************************/
/*                                                                   */
/*          Module name: alias_module.h                              */
/*          Description:                                             */
/*              This module provides structures and functions to     */
/*              store and remove aliases.                            */
/*                                                                   */
/*********************************************************************/


#ifndef ALIAS_MODULE_H 
#define ALIAS_MODULE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "string_module.h"

#define ALIAS_LIMIT 250 
#define VALUE_LIMIT 150
#define NAME_LIMIT 50

/* structure to hold alias values */
typedef struct alias_t
{
    char    name[NAME_LIMIT];
    char    value[VALUE_LIMIT];
} alias;

/* prototypes */
int     add_alias( char*, char* );
int     remove_alias( const char* );
alias*  find_alias( const char* );
void    adjust_aliases( alias* );
void    print_aliases( void );
int     alias_cmp( const void*, const void* );

#endif
