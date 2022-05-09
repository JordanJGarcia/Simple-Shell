#include "alias_module.h"

/* globals */
int     n_aliases = 0;

/*********************************************************************/
/*                                                                   */
/*      Function name: add_alias                                     */
/*      Return type:   int (SUCCESS/FAILURE)                         */
/*      Parameter(s):                                                */
/*          char* name:  alias name                                  */
/*          char* value: alias value                                 */
/*                                                                   */
/*********************************************************************/
int add_alias( char* name, char* value )
{
    int i;

    // check that we haven't reached max amount of aliases
    if ( n_aliases == ALIAS_LIMIT )
    {
        fprintf( stderr, "Error: Maximum number of allowed aliases reached.\n" );
        return FAILURE;
    }

    // check if alias already exists 
    if ( find_alias( name ) != NULL )
    {
        fprintf( stderr, "Error: Alias already exists.\n" );
        return FAILURE;
    }

    // check that alias name & value is within limit
    if ( strlen( name ) >= NAME_LIMIT )
    {
        fprintf( stderr, "Error: alias name is too large." );
        return FAILURE;
    }
    else if ( strlen( value ) >= VALUE_LIMIT )
    {
        fprintf( stderr, "Error: alias value is too large." );
        return FAILURE;
    }

    // copy alias name into structure
    strcpy( aliases[n_aliases].name, name );

    // copy alias value into structure
    strcpy( aliases[n_aliases].value, value );

    // sort array for bsearch() & increment n_aliases
    qsort( aliases, (size_t)(++n_aliases), sizeof(aliases[0]), alias_cmp );

    return SUCCESS; 
} /* end add_alias() */


/*********************************************************************/
/*                                                                   */
/*      Function name: remove_alias                                  */
/*      Return type:   int (SUCCESS/FAILURE)                         */
/*      Parameter(s):                                                */
/*          const char* a: alias name to remove                      */
/*                                                                   */
/*********************************************************************/
int remove_alias( const char* a )
{
    // search for alias
    alias* found = find_alias( a );

    // check that alias exists
    if ( found == NULL )
    {
        fprintf( stderr, "Error. Alias does not exist.\n" );
        return FAILURE;
    }

    // remove this alias
    alias* dummy = found; 
    adjust_aliases( dummy );
    
    // sort array for bsearch() & decrement n_aliases
    qsort( aliases, (size_t)(--n_aliases), sizeof(aliases[0]), alias_cmp );

    return SUCCESS;
} /* end remove_alias() */


/*********************************************************************/
/*                                                                   */
/*      Function name: find_alias                                    */
/*      Return type:                                                 */
/*          alias* : location of found alias, NULL if not found      */
/*      Parameter(s):                                                */
/*          const char* a: alias name to find                        */
/*                                                                   */
/*********************************************************************/
alias* find_alias( const char* a )
{

    alias* found = NULL;
    found = (alias*)bsearch( a, aliases, (size_t)n_aliases, 
                             sizeof( aliases[0] ), alias_cmp );

    return found;
} /* end find_alias() */


/*********************************************************************/
/*                                                                   */
/*      Function name: adjust_aliases                                */
/*      Return type:   none                                          */
/*      Parameter(s):                                                */
/*          alias* found: pointer to the type alias struct we are    */
/*                        removing. i.e. the start point             */
/*                                                                   */
/*      Description:                                                 */
/*          Adjusts the array of aliases when removing an alias by   */
/*          moving all later alias instances down an index.          */
/*                                                                   */
/*********************************************************************/
void adjust_aliases( alias* found )
{
    int i;

    // move all values down one index in the array
    for ( ; found != &aliases[n_aliases-1]; found++ )
    {
        // copy next struct to this one (shifting them down)
        strcpy( found->name, ( found + 1 )->name );
        strcpy( found->value, ( found + 1 )->value );
    }
    
    // clear last alias values
    memset( found->name, 0, NAME_LIMIT );
    memset( found->value, 0, VALUE_LIMIT );

    return;
} /* end adjust_aliases */

/*********************************************************************/
/*                                                                   */
/*      Function name: print_aliases                                 */
/*      Return type:   void                                          */
/*      Parameter(s):  none                                          */
/*                                                                   */
/*********************************************************************/
void print_aliases( void )
{
    if ( n_aliases == 0 )
    {
        puts( "No aliases have been created." );
        return;
    }

    int ctr;

    for( ctr = 0; ctr < n_aliases; ctr++ )
        printf( "%s\t%s\n", aliases[ctr].name, aliases[ctr].value );

    return;
} /* end print_aliases */


/*********************************************************************/
/*                                                                   */
/*      Function name: alias_cmp                                     */
/*      Return type:   int                                           */
/*      Parameter(s):                                                */
/*          a1: pointer to first alias.                              */
/*          a2: pointer to second alias.                             */
/*      Description:                                                 */
/*          used to compare aliases for qsort and bsearch.           */
/*                                                                   */
/*********************************************************************/
int alias_cmp( const void* a1, const void* a2 )
{
    return strcmp( ((const alias*)a1)->name, ((const alias*)a2)->name );
} /* end alias_cmp() */
