#include "history_module.h"

/* globals */
int     n_history = 0;
char*   history[HISTORY_LIMIT];

/*********************************************************************/
/*                                                                   */
/*      Function name: add_cmds_to_history                           */
/*      Return type:   int                                           */
/*      Parameter(s):                                                */
/*          char* cmds:   command to add to history array            */
/*                                                                   */
/*********************************************************************/
int add_to_history( char* line )
{
    // if history array is maxed out, write to a file
    if ( n_history == CMD_LIMIT )
        write_history_to_file();

    // allocate memory for commands
    if ( ( history[n_history] = (char*)malloc( ( strlen(line) + 1 ) * sizeof(char) ) ) == NULL )
    {
        fprintf( stderr, "Error allocating memory for history." );
        return FAILURE; 
    }

    // copy line into history array
    strcpy( history[n_history++], line );

    return SUCCESS;
} /* end add_cmds_to_history() */


/*********************************************************************/
/*                                                                   */
/*      Function name: print_history                                 */
/*      Return type:   void                                          */
/*      Parameter(s):  FILE* - a pointer to the file stream we want  */
/*                             to write the history to.              */
/*                                                                   */
/*********************************************************************/
void print_history( FILE *fp )
{
    int i;
    for( i = 0; i < n_history; i++ )
        fprintf( fp, "%d\t%s\n", i + 1, history[i] );

} /* end print_history() */


/*********************************************************************/
/*                                                                   */
/*      Function name: write_history_to_file                         */
/*      Return type:   int                                           */
/*      Parameter(s): none                                           */
/*                                                                   */
/*********************************************************************/
int write_history_to_file( void )
{
    char out_file[255];
    sprintf( out_file, "%s%s", 
        ( getenv( "HOME" ) == NULL ? ".": getenv( "HOME" ) ), "/.j_history" ); 

    char* mode = "a+";
    FILE* fp; 

    // open file to write to
    if( ( fp = fopen( out_file, mode ) ) == NULL )
    {
        fprintf( stderr, "Error. Could not open %s to write history.\n", out_file );
        perror("Error");
        return FAILURE;
    }

    // write all history to outfile and free the memory
    print_history( fp ); 
    free_history();

    // close file
    fclose( fp );

    return SUCCESS; 
} /* end write_history_to_file() */


/*********************************************************************/
/*                                                                   */
/*      Function name: free_history                                  */
/*      Return type:   int                                           */
/*      Parameter(s):  none                                          */
/*      Description:                                                 */
/*          Frees all memory allocated for history array.            */
/*                                                                   */
/*********************************************************************/
int free_history( void )
{
    int i; 
    for( i = 0; i < n_history; i++ )
        free( history[i] );

    n_history = 0;

    return SUCCESS; 
} /* end free_history() */
