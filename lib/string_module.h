/*********************************************************************/
/*                                                                   */
/*      Module for manipulating strings and string arrays            */
/*                                                                   */
/*                                                                   */
/*********************************************************************/

#ifndef STRING_MOD_H
#define	STRING_MOD_H

/* libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* macros */
#define FAILURE 0
#define SUCCESS 1
#define MAX_CMDS 250
#define WORD_LIMIT 255
#define T 1
#define F 0

/* function prototypes */
int 	parse_string( char* line, char* cmds[], int* count );
int     add_string( char* str, char* arr[], int index );
int     merge_string_arrays( char* dest[], int dest_cnt, char* src[], int src_cnt, int index );
int     shift_strings_down( char* arr[], int* arr_cnt, int index, int amount );

/* static function prototypes */
static int     is_special_char( const char c );
static void    save_word( char* cmd, char* cmds[], int* pos, int* cmd_len );   

#endif
