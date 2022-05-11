/*********************************************************************/
/*                                                                   */
/*      Author: Jordan Garcia                                        */
/*      Date:   04/18/2022                                           */
/*      Project:                                                     */
/*                                                                   */
/*          This project is my implementation of a simple shell.     */
/*                                                                   */
/*********************************************************************/

// IDEAS:
    // add prompt customization feature
    // add ability to run jobs in background
    // add wildcard matching

// NEXT STEPS:
    // 1) Revise documentation to make sure every thing is accurate
    // 3) fix any ineffecient areas of the system
    // 4) add ability to read in permanent aliases from .j_profile
    // 5) add any other features you think of! :D
    // 6) make sure memory leaks don't exist

    // CLEARLY DEFINE WHAT A WORD IS INTERPRETTED AS BY THE PROGRAM IN THE README
    // state how input/output redirects work CLEARLY - can only do one at a time or both at once, but cannot output redirect twice or input redirect twice in one command

// standard libraries 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

// for custom libraries 
#include "../lib/alias_module.h"
#include "../lib/string_module.h"
#include "../lib/history_module.h"
#include "../lib/execution_module.h"

// macros
#define PROMPT_SIZE 255
#define FAILURE 0
#define SUCCESS 1
#define PWD "PWD"
#define USER "USER"
#define HOST "HOST"

// global variables 
char*   cmds[MAX_CMDS]; 
int     n_cmds = 0; 
char    previous_dir[WORD_LIMIT] = "";

// utility function prototypes 
void    start_shell( void );
int     process_commands( void );

// history handling 
int     handle_history( void );

// alias handling 
int     handle_aliases( void );
int     check_for_alias( void );

// env variable handling 
int     handle_env_vars( void );
int     convert_env_var( int index );
int     check_for_var_in_quotes( int i );

// directory change handling 
int     handle_directory_change( void );
int     change_to_home_dir( void );
int     change_to_prev_dir( void );
int     change_dir( void );
int     translate_dir_path( int loc );

// program execution function prototypes 
int     handle_program_execution( void );

// helper function (low level) 
int     is_directory( const char* dirname );
int     is_reg_file( const char* filename );
int     is_redirect( const char type );
void    print_commands( void );
int     count_pipes( void );


/*********************************************************************/
/*                                                                   */
/*      Function name: main()                                        */
/*      Return type:   int                                           */
/*      Parameter(s):  None                                          */
/*      Description:                                                 */
/*          main() will start the shell.                             */
/*                                                                   */
/*********************************************************************/
int main( void )
{
    start_shell();
    return EXIT_SUCCESS;
} /* end main */


/*********************************************************************/
/*                                                                   */
/*      Function name: start_shell                                   */
/*      Return type:   void                                          */
/*      Parameter(s):  None                                          */
/*      Description:                                                 */
/*          start_shell() will begin a loop prompting the user       */
/*          for commands and call functions to tokenize and process  */
/*          those commands.                                          */
/*                                                                   */
/*********************************************************************/
void start_shell( void )
{
    char prompt[PROMPT_SIZE];
    char* line = NULL;
    int i;
    
    // initialize cmds to NULL
    for( i = 0; i < MAX_CMDS; i++ )
        cmds[i] = NULL;

    // begin infinite loop that is the shell 
    while ( 1 )
    {
        // set prompt to display proper information each time
        if( getenv( HOST ) == NULL )
            sprintf( prompt, "\033[1;36m%s\033[1;35m@\033[1;33mUnknownHost \033[1;32m[%s]> \033[0m", getenv( USER ), getenv( PWD ) );
        else
            sprintf( prompt, "\033[1;36m%s\033[1;35m@\033[1;33m%s \033[1;32m[%s]> \033[0m", getenv( USER ), getenv( HOST ), getenv( PWD ) );

        // prompt then read line - line is allocated with malloc(3) 
        line = readline(prompt);

        // check if user wants to exit the shell 
        if ( strcmp( line, "exit" ) == 0 )
        {
            free( line );
            free_history();
            return;
        }
        else if( parse_string( line, cmds, &n_cmds ) == FAILURE )
            ; 
        else if(n_cmds > 0)
            process_commands();

        // add command to history
        add_to_history( line );

        // free all memory and reset n_cmds
        for ( int i = 0; i < n_cmds; i++ )
        {
            free( cmds[i] );
            cmds[i] = NULL;
        }
        free( line );
        n_cmds = 0;
    }

    return;
} /* end start_shell() */


/*********************************************************************/
/*                                                                   */
/*      Function name: process_commands                              */
/*      Return type:   int                                           */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*      Description:                                                 */
/*          Handles parsed commands for appropriate processing.      */
/*                                                                   */
/*********************************************************************/
int process_commands( void )
{
    /* error checking */
    if( n_cmds == 0 )
    {
        fprintf( stderr, "No commands to process.\n" );
        return FAILURE;
    }

    // handle printing of history
    if( handle_history() == SUCCESS )
        return SUCCESS; 

    // handle all alias processing 
    if( handle_aliases() == FAILURE )
        return FAILURE;

    // handle environmental variable translations
    if( handle_env_vars() == FAILURE )
        return FAILURE;

    // handle directory changes
    if( handle_directory_change() == SUCCESS )
        return SUCCESS;
        
    // handle program execution
    return handle_program_execution();
}/* end process_commands */


/*********************************************************************/
/*                                                                   */
/*      Function name: handle_history                                */
/*      Return type:   int                                           */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*      Description:                                                 */
/*          prints history of commands entered.                      */
/*                                                                   */
/*********************************************************************/
int handle_history( void )
{
    if ( strcmp( cmds[0], "history" ) == 0 )
    {
        print_history( stdout ); 
        return SUCCESS;
    }
    return FAILURE;
} /* end handle_history() */


/*********************************************************************/
/*                                                                   */
/*      Function name: check_for_alias                               */
/*      Return type:   int                                           */
/*      Description:                                                 */
/*          Checks for aliases and converts them if found.           */
/*          Return failure anytime an alias isnt being translated.   */
/*          This is done so that way we know when we dont need to    */
/*          use the alias any further.                               */
/*                                                                   */
/*********************************************************************/
int handle_aliases( void )
{
    // case found keyword "alias"
    if ( strcmp( cmds[0], "alias" ) == 0 )
    {
        // if alias is only command
        if( n_cmds == 1 )
        {
            print_aliases();
            return FAILURE;
        }
        else if( n_cmds == 2 )
        {
            // if we want to display a specified alias
            alias* specified = NULL;

            // search for specified alias
            if( ( specified = find_alias( cmds[1] ) ) == NULL )
            {
                fprintf( stderr, "Error: could not find alias - %s\n" , cmds[1] );
                return FAILURE;
            }
            
            // show specified alias
            printf( "%s\t%s\n", specified->name, specified->value );
            return FAILURE;
        }
        else if( n_cmds == 4 ) 
        {
            // if adding alias, parser should separate into 4 commands
            if( add_alias( cmds[1], cmds[3] ) == FAILURE )
                fprintf( stderr, "Error: could not add alias %s\n", cmds[3] );
    
            return FAILURE;
        }
            
        // if it does not match above patterns, return
        fprintf( stderr, "Error: no alias specified to add.\n" );
        return FAILURE;
    }
    else if ( strcmp( cmds[0], "unalias" ) == 0 )
    {
        // make sure user provided an alias name
        if( n_cmds < 2 )
        {
            fprintf( stderr, "Error: no alias specified to remove.\n" );
            return FAILURE;
        }
        remove_alias( cmds[1] );
        return FAILURE;
    }
    else
        check_for_alias();    

    return SUCCESS; 
} /* end handle_aliases() */


/*********************************************************************/
/*                                                                   */
/*      Function name: handle_env_vars                               */
/*      Return type:   int                                           */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*      Description:                                                 */
/*          Checks for environmental variables and calls conversion  */
/*          function.                                                */
/*                                                                   */
/*********************************************************************/
int handle_env_vars( void )
{
    int i;

    // go through cmds looking for an environmental variable
    for( i = 0; i < n_cmds; i++ )
    {
        // if we find an environmental variable - not in quotes
        if( strcmp( cmds[i], "$" ) == 0 )
        {
            // if conversion fails
            if ( convert_env_var( i ) == FAILURE )
            {
                fprintf( stderr, "Error: could not convert environtmental variable - %s\n", cmds[i + 1] );
                return FAILURE;
            }

            // cmds have been shifted down, so account for it
            i--; 
        } 
        else // check for possible environmental variables in quoted cmd
            check_for_var_in_quotes( i );
    }
    return SUCCESS; 
} /* end handle_env_vars() */


/*********************************************************************/
/*                                                                   */
/*      Function name: handle_directory_change                       */
/*      Return type:   int                                           */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*      Description:                                                 */
/*          Determines and conducts directory change if needed.      */
/*                                                                   */
/*********************************************************************/
int handle_directory_change( void )
{
    // ensure we want to switch directories
    if ( strcmp( cmds[0], "cd" ) != 0 )
        return FAILURE;

    // switching to home directory 
    if ( n_cmds == 1 || ( n_cmds == 2 && ( strcmp( cmds[1], "~/" ) == 0 || strcmp( cmds[1], "~" ) == 0 ) ) )
        return change_to_home_dir();

    // switching to previous directory
    if ( n_cmds == 2 && strcmp( cmds[1], "-" ) == 0 )
        return change_to_prev_dir();

    // switching to any other directory 
    return change_dir();
} /* end handle_directory_change() */


/*********************************************************************/
/*                                                                   */
/*      Function name: handle_program_execution                      */
/*      Return type:   int                                           */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*      Description:                                                 */
/*          Routes all program execution to their respected          */
/*          functions.                                               */
/*                                                                   */
/*********************************************************************/
int handle_program_execution( void )
{
    // set up input/output redirects and pipes
    int i, j = 0, n_pipes = count_pipes(), outfile = is_redirect( 'o' ), infile = is_redirect( 'i' );
    int pipe_index[n_pipes];

    // store the locations of each pipe
    // this is a bit redundant but I will improve later
    for( i = 0; i < n_cmds; i++ )
    {
        if( strcmp( cmds[i], "|" ) == 0 )
           pipe_index[j++] = i;  
    }

    // execute command
    execute( infile, outfile, n_pipes, pipe_index );
   
    return SUCCESS;
} /* end handle_program_execution() */


/*********************************************************************/
/*                                                                   */
/*      Function name: check_for_alias                               */
/*      Return type:   void                                          */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*      Description:                                                 */
/*          checks for aliases and converts them if found            */
/*                                                                   */
/*********************************************************************/
int check_for_alias( void ) 
{
    alias* found;
    char* alias_cmds[MAX_CMDS];
    int i, j, n_alias_cmds = 0;

    // go through commands
    for( i = 0; i < n_cmds; i++ )
    {
        // check if command is an alias
        if( ( found = find_alias( cmds[i] ) ) != NULL )
        {
            // parse alias value
            if( parse_string( found->value, alias_cmds, &n_alias_cmds ) == FAILURE )
            {
                fprintf( stderr, "Error: could not parse alias value\n" );
                return FAILURE;
            }

            // place alias value into cmds[]
            if( merge_string_arrays( cmds, n_cmds, alias_cmds, n_alias_cmds, i ) == FAILURE )
            {
                fprintf( stderr, "Error: could not merge alias value into cmds\n" );
                return FAILURE;
            }

            // adjust n_cmds
            n_cmds += ( n_alias_cmds - 1 );

            // clear alias_cmds and reset n_alias_cmds in case we find more aliases
            for( j = 0; j < n_alias_cmds; j++ )
            {
                free( alias_cmds[j] );
                alias_cmds[j] = NULL;
            }
            n_alias_cmds = 0;
        }
    }
    return SUCCESS; 
} /* end check_for_alias() */


/*********************************************************************/
/*                                                                   */
/*      Function name: check_for_var_in_quotes                       */
/*      Return type:   void                                          */
/*      Parameter(s):                                                */
/*          int i: the index of the str in cmds[] we are searching   */
/*                                                                   */
/*      Description:                                                 */
/*          checks for env variables inside of a string,             */
/*          it is used to search through quoted commands for         */
/*          env vars                                                 */
/*                                                                   */
/*********************************************************************/
int check_for_var_in_quotes( int i )
{
    char* cur_env_loc = NULL, * prev_env_loc = NULL, * translated_env_var = NULL; 
    char env_var[WORD_LIMIT] = "", buffer[WORD_LIMIT] = "";
    int j;

    // search for an env var indicator in the quoted string ($)
    if( ( cur_env_loc = strstr( cmds[i], "$" ) ) != NULL )
    {
        // copy everything up to env var into buffer
        strncpy( buffer, cmds[i], cur_env_loc - cmds[i] );

        // gather, convert, and append env var
        do 
        {
            // env variables don't start with digits
            if( isdigit( *(cur_env_loc++) ) )
            {
                prev_env_loc = cur_env_loc;
                continue; 
            }

            // copy characters in between env vars if found multiple
            if( cur_env_loc != prev_env_loc && prev_env_loc != NULL )
                strncat( buffer, prev_env_loc, (int)(cur_env_loc - prev_env_loc - 1) );

            // clear out env_var[]
            memset( env_var, 0, WORD_LIMIT * sizeof( char ) );

            // get the var name
            for( j = 0; !isspace( *cur_env_loc ) && !ispunct( *cur_env_loc ) && !isdigit( *cur_env_loc ) && *cur_env_loc != '\0' && *cur_env_loc != '$'; j++ )
                env_var[j] = *(cur_env_loc++); 

            // translate env var & append to buffer
            if( ( translated_env_var = getenv( env_var ) ) == NULL )
            {
                fprintf( stderr, "Error: could not find env var $%s\n", env_var );
                return FAILURE;
            }
            else if( strlen( buffer ) + strlen( translated_env_var ) < WORD_LIMIT )
                strcat( buffer, translated_env_var );
            else
            {
                fprintf( stderr, "Error: command too large after appending environmental variable $%s\n", env_var );
                fprintf( stderr, "       command before attempt to append: %s\n", buffer );
                return FAILURE;
            }

            // store end location of var name
            prev_env_loc = cur_env_loc;
        }
        while( ( cur_env_loc = strstr( prev_env_loc, "$" ) ) != NULL );

        // copy the remainder of cmds[i] into buffer
        if( strlen( buffer ) + strlen( &cmds[i][prev_env_loc - cmds[i]] ) < WORD_LIMIT )
            strcat( buffer, &cmds[i][prev_env_loc - cmds[i]] );
        else
        {
            fprintf( stderr, "Error: command too large after appending last part (%s)\n", &cmds[i][prev_env_loc - cmds[i]] );
            return FAILURE;
        }

        // replace cmds[i] with buffer
        free( cmds[i] );
        add_string( buffer, cmds, i );
    }

    return SUCCESS; 
} /* end check_for_var_in_quotes() */


/*********************************************************************/
/*                                                                   */
/*      Function name: convert_env_var                               */
/*      Return type:   int                                           */
/*      Parameter(s):                                                */
/*          int index: the index in cmds that we found the env_var   */
/*                     indicator ($)                                 */
/*                                                                   */
/*      Description:                                                 */
/*          checks for environmental variables and converts cmds[]   */
/*          so that it has variable value (must not be in quotes)    */
/*                                                                   */
/*********************************************************************/
int convert_env_var( int index )
{
    // get variable value
    char* env_var = getenv( cmds[index + 1] );
    
    // case env var not found, exit gracefully
    if( env_var == NULL )
    {
        fprintf( stderr, "Error: could not translate environmental variable - %s\n", cmds[index + 1] );
        return FAILURE; 
    }

    // remove variable name
    free( cmds[index + 1] );

    // replace with variable value
    if( add_string( env_var, cmds, index + 1 ) == FAILURE )
    {
        fprintf( stderr, "Error: could not place environmental variable in cmds - %s\n", env_var );
        return FAILURE;
    }
    
    // shift all cmds down by one, starting at index + 1
    if( shift_strings_down( cmds, &n_cmds, index + 1, 1 ) == FAILURE )
    {
        fprintf( stderr, "Error: convert_env_var() - could not shift cmds to remove '$'\n" );
        return FAILURE;
    }
    return SUCCESS;
} /* end convert_env_var() */


/*********************************************************************/
/*                                                                   */
/*      Function name: change_to_home_dir                            */
/*      Return type:   int                                           */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*      Description:                                                 */
/*          Attempts to change directories to the user's $HOME dir   */
/*                                                                   */
/*********************************************************************/
int change_to_home_dir( void )
{
    // check that $HOME has a valid directory value
    if( is_directory( getenv( "HOME" ) ) )
    {
        // change to $HOME directory
        if( chdir( getenv( "HOME" ) ) != 0 )
        {
            printf("Error: Cannot switch to home directory.\n" );
            return FAILURE;
        }
        else
        {
            // set previous_dir
            strcpy( previous_dir, getenv( PWD ) );

            // set $PWD 
            setenv( PWD, getenv( "HOME" ), 1 );

            // display new directory
            puts( getenv( PWD ) );
        }
    }
    return SUCCESS;
} /* end change_to_home_dir() */


/*********************************************************************/
/*                                                                   */
/*      Function name: change_to_prev_dir                            */
/*      Return type:   int                                           */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*      Description:                                                 */
/*          Attempts to change directories to the previous dir       */
/*                                                                   */
/*********************************************************************/
int change_to_prev_dir( void )
{
    // change to previous dir
    if ( chdir( previous_dir ) != 0 )
    {
        fprintf( stderr, "Error: Cannot change to previous directory - %s\n", previous_dir );
        return FAILURE;   
    }

    // set temporary holding spot for previous_dir
    char temp_prev_dir[WORD_LIMIT];
    strcpy( temp_prev_dir, getenv( PWD ) );

    // change $PWD 
    setenv( PWD, previous_dir, 1 );

    // set previous_dir
    strcpy( previous_dir, temp_prev_dir );

    // display new directory
    puts( getenv( PWD ) );

    return SUCCESS;    
} /* end change_to_prev_dir() */


/*********************************************************************/
/*                                                                   */
/*      Function name: change_dir                                    */
/*      Return type:   int                                           */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*      Description:                                                 */
/*          Attempts to change directories to user specified dir     */
/*                                                                   */
/*********************************************************************/
int change_dir( void )
{
    char* found = NULL, abs_path[WORD_LIMIT];

    if( n_cmds == 2 )
    {   
        // remove last '/' in cmds[1]
        if( cmds[1][strlen( cmds[1] ) - 1] == '/' )
            cmds[1][strlen( cmds[1] ) - 1] = '\0';

        // translate dir path if user provider ".." in cmd
        while( ( found = strstr( cmds[1], ".." ) ) != NULL )
        {
            if( translate_dir_path( (int)( found - cmds[1] ) ) == FAILURE )
            {
                fprintf( stderr, "Error: could not translate directory path provided.\n" );
                return FAILURE;
            }
        }
    
        // if we are changing to relative dir, make it an absolute from $PWD
        if( cmds[1][0] != '/' )
        {
            sprintf( abs_path, "%s/%s", getenv( PWD ), cmds[1] );
            free( cmds[1] );
            add_string( abs_path, cmds, 1 );
        }

        // switch to directory user provided
        if ( chdir( cmds[1] ) != 0 )
        {
            fprintf( stderr, "Error: Cannot change directory to %s\n", cmds[1] );
            return FAILURE;
        } 
        
        // set previous_dir
        strcpy( previous_dir, getenv( PWD ) );

        // change $PWD
        setenv( PWD, cmds[1], 1 );
    
        // display new directory
        puts( getenv( PWD ) );
    }
    else
    {
        fprintf( stderr, "Error: directory not provided\n" );
        return FAILURE;
    }
    return SUCCESS;
} /* end change_dir() */


/*********************************************************************/
/*                                                                   */
/*      Function name: translate_dir_path                            */
/*      Return type:   int                                           */
/*      Parameter(s):                                                */
/*          int loc: location of first occurence of ".." in user     */
/*                   provided path                                   */
/*                                                                   */
/*      Description:                                                 */
/*          modifies the user provided directory path in cmds[1]     */
/*          in the case that the user wishes to cd using ".."        */
/*                                                                   */
/*********************************************************************/
int translate_dir_path( int loc )
{
    char new_path[WORD_LIMIT] = "", buffer[WORD_LIMIT] = "";
    char* last_dir_loc = NULL;

    // case the user provided path starts with ".."
    if( loc == 0 )
        strcpy( buffer, getenv( PWD ) );
    else // case the ".." is later in the path
        strncpy( buffer, cmds[1], loc - 1 );

    // check if we are already in root directory
    if( strcmp( buffer, "/" ) == 0 )
    {
        fprintf( stderr, "Error: you have reached the root of the tree, further down you cannot dig\n" );
        return FAILURE;
    }

    // copy up to last occurence of "/" 
    if( ( last_dir_loc = strrchr( buffer, '/' ) ) != NULL )
        strncpy( new_path, buffer, (size_t)( last_dir_loc - buffer == 0 ? 1 : last_dir_loc - buffer ) );

    // append the rest of the user provided path if theres more
    if( strlen( cmds[1] ) - 1 > loc + 2 )
        strcat( new_path, &cmds[1][loc + 2] );

    // reset user provided cmd
    free( cmds[1] );
    add_string( new_path, cmds, 1 );

    return SUCCESS;
}

/*********************************************************************/
/*                                                                   */
/*                    HELPER FUNCTIONS (LOW LEVEL)                   */
/*                                                                   */
/*********************************************************************/


/*********************************************************************/
/*                                                                   */
/*      Function name: is_redirect                                   */
/*      Return type:   char* - name of file being redirected to      */
/*      Parameter(s):                                                */
/*          const char type: type of redirection ('i' for input,     */
/*                           'o' for output)                         */
/*                                                                   */
/*      Description:                                                 */
/*          returns the name of the file we are redirecting input    */
/*          or output to.                                            */
/*                                                                   */
/*********************************************************************/
int is_redirect( const char type )
{
    if( type != 'i' && type != 'o' )
    {
        fprintf( stderr, "Error: incorrect type specified in is_redirect()\n" );
        fprintf( stderr, "       valid types are 'i' or 'o'\n" );
        return -1;
    }

    char* io = ( type == 'i' ? "<" : ">" );
    int i = 0;

    for ( ; i < n_cmds - 1; i++ )
    {
        if ( strcmp( cmds[i], io ) == 0 )
            return i + 1;
    }

    return -1; 
} /* end is_redirect() */



/*********************************************************************/
/*                                                                   */
/*      Function name: print_commands                                */
/*      Return type:   void                                          */
/*      Parameter(s):  None                                          */
/*                                                                   */
/*      Description:                                                 */
/*          Prints the array of strings containing the parsed        */
/*          command line.                                            */
/*                                                                   */
/*********************************************************************/
void print_commands( void )
{
    puts(" ");
    for ( int i = 0; i <= n_cmds; i++ )
        printf( "command %d: %s\n", i, cmds[i] );

    return;
} /* end print_commands() */


/*********************************************************************/
/*                                                                   */
/*      Function name: is_directory                                  */
/*      Return type:   int                                           */
/*      Parameter(s):  1                                             */
/*              const char*: directory name                          */
/*                                                                   */
/*      Description:                                                 */
/*          determines if dirname is a directory                     */
/*                                                                   */
/*********************************************************************/
int is_directory( const char* dirname )
{
    struct stat buffer;
    if( stat( dirname, &buffer ) != 0 )
        return 0;
    
    return S_ISDIR( buffer.st_mode );
} /* end is_directory() */


/*********************************************************************/
/*                                                                   */
/*      Function name: file_exists                                   */
/*      Return type:   int                                           */
/*      Parameter(s):  1                                             */
/*              const char*: file name                               */
/*                                                                   */
/*      Description:                                                 */
/*          determines if file exists                                */
/*                                                                   */
/*********************************************************************/
int is_reg_file( const char* filename )
{
    struct stat buffer;
    if( stat( filename, &buffer ) != 0 )
        return 0;

    return S_ISREG( buffer.st_mode );
} /* end is_reg_file() */


/*********************************************************************/
/*                                                                   */
/*      Function name: count_pipes                                   */
/*      Return type:   void                                          */
/*      Parameter(s):  None                                          */
/*                                                                   */
/*      Description:                                                 */
/*          counts the number of pipes in the current command        */
/*                                                                   */
/*********************************************************************/
int count_pipes( void )
{
    int i, n_pipes = 0;
    for( i = 0; i < n_cmds; i++ )
    {
        if( strcmp( cmds[i], "|" ) == 0 )
            n_pipes += 1;
    }
    return n_pipes; 
} /* end count_pipes() */
