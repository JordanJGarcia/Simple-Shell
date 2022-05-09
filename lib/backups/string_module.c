#include "string_module.h"


/*********************************************************************/
/*                                                                   */
/*      Function name: parse_string                                  */
/*      Return type:   int                                           */
/*      Parameter(s):                                                */
/*          char* line: line of commands user types into shell       */
/*          char* cmds[]: array of strings to place each command in  */
/*          int* count: pointer to var holding the count in cmds[]   */
/*                                                                   */
/*      Description:                                                 */
/*          This function splits a string into an array              */
/*          of strings, each string in the array representing        */
/*          one of the words in the command line                     */
/*          A pointer to the int representing the count of commands  */
/*          in the array of strings is passed so we can update how   */
/*          many commands are stored in the array                    */
/*                                                                   */
/*********************************************************************/
int parse_string( char* line, char* cmds[], int* count )
{
    // cmd stores each individual word within the string "line"
    char cmd[WORD_LIMIT] = "";
    int i, idx = 0, end_of_line = strlen( line ) - 1;

    // go through entire line
    for( i = 0; i <= end_of_line; i++ )
    {
        // bounds checking
        if( idx == WORD_LIMIT - 1 )
        {
            fprintf( stderr, "Error: parser detected word larger than %d characters\n", WORD_LIMIT );
            fprintf( stderr, "       first %d characters of word in question: %s\n", WORD_LIMIT, cmd );
            return FAILURE;
        }

        // special character check
        if ( is_special_char( line[i] ) )
        {
            // save word
            save_word( cmd, cmds, count, &idx );

            // add the special char as a word to the arr
            cmd[idx++] = line[i];
        
            save_word( cmd, cmds, count, &idx );
        }
        else if ( i == end_of_line ) // end of line
        {
            // if its not a space, append char to cmd
            if( !isspace( line[i] ) )
                cmd[idx++] = line[i];

            // add final word to cmds[]
            save_word( cmd, cmds, count, &idx );
        }
        else if ( line[i] == '\"' || line[i] == '\'' ) // string in quotes
        {
            char end_quote = line[i++];

            // build command with everything inside quotes
            do
            {
                // bounds checking
                if( idx == WORD_LIMIT - 1 )
                {
                    fprintf( stderr, "Error: parser detected word larger than %d characters\n", WORD_LIMIT );
                    fprintf( stderr, "       first %d characters of word in question: %s\n", WORD_LIMIT, cmd );
                    return FAILURE;
                }

                // check if user forgot end quote
                if( i == end_of_line )
                {
                    cmd[idx++] = line[i]; 
                    break;
                }

                cmd[idx++] = line[i];
            }
            while( line[++i] != end_quote );

            // add entire quoted string as one command
            save_word( cmd, cmds, count, &idx );
        }
        else if ( isspace( line[i] ) ) // if space, save word
            save_word( cmd, cmds, count, &idx );
        else // everything else, build the word 
            cmd[idx++] = line[i];
    }
    cmds[*count] = NULL;

    return SUCCESS;
} /* end split_input() */


/*********************************************************************/
/*                                                                   */
/*      Function name: add_string                                    */
/*      Return type:   int                                           */
/*      Parameter(s):                                                */
/*          char* str: pointer to string we are adding               */
/*          char* arr[]: pointer(address) of the array of strings    */
/*                       we are adding str to                        */
/*          int* count: pointer to the count of commands in arr[]    */
/*                                                                   */
/*      Description:                                                 */
/*          adds a string to an array of pointers to char.           */
/*                                                                   */
/*********************************************************************/
int add_string( char* str, char* arr[], int index )
{
    // check that str has data
    if ( str == NULL )
        return SUCCESS;

    // do bounds checking
    if( index >= MAX_CMDS )
    {
        // exit gracefully
        fprintf( stderr, "Error: command too large. Commands should be %d words or less\n", MAX_CMDS );
        return FAILURE;
    }

    // allocate memory for str in the array
    if ( ( arr[index] = (char*)malloc( ( strlen(str) + 1 ) * sizeof(char) ) ) == NULL )
    {
        fprintf( stderr, "Error: could not allocate memory for word in arr[] - %s\n", str );
        return FAILURE;
    }

    // copy the string to the array
    strcpy( arr[index], str );

    return SUCCESS;
}/* end add_string() */


/*********************************************************************/
/*                                                                   */
/*      Function name: merge_string_arrays                           */
/*      Return type:   int                                           */
/*      Parameter(s):                                                */
/*          char* dest[]: array of strings we are adding src[] to    */
/*          int* dest_cnt: count of strings in dest[]                */
/*          char* src[]: array of strings we are merging into dest[] */
/*          int src_cnt: count of strings in src[]                   */
/*          int index: location in dest that we want to add src to   */
/*                                                                   */
/*      Description:                                                 */
/*          merges one array into the other, user should perform     */
/*          adequate bounds checking prior to calling this.          */
/*                                                                   */
/*********************************************************************/
int merge_string_arrays( char* dest[], int dest_cnt, char* src[], int src_cnt, int index )
{
    // check that src has commands
    if ( src_cnt == 0 )
        return SUCCESS;

    int i, j, loc = (dest_cnt - 1) + (src_cnt - 1);

    // start at the end of dest[], shifting every word up src_cnt indices
    for( i = dest_cnt - 1; i > index; i--, loc-- )
    {
        if( add_string( dest[i], dest, loc ) == FAILURE )
            return FAILURE;
    }

    // starting at index, add the strings in src[] to dest[]
    for( i = index, j = 0; i < index + src_cnt; i++, j++ )
    {
        // free memory used from before
        if( i < dest_cnt )
            free( dest[i] );

        // copy alias into cmds
        if( add_string( src[j], dest, i ) == FAILURE )
        {
            fprintf( stderr, "Error: could not add %s to cmds\n", src[j] );
            return FAILURE;
        }
    }
    
    return SUCCESS;
}/* end merge_string_arrays() */


/*********************************************************************/
/*                                                                   */
/*      Function name: shift_strings_down                            */
/*      Return type:   int                                           */
/*      Parameter(s):                                                */
/*          char* arr[]: array of strings we are shifting            */
/*          int* arr_cnt: count of strings in arr[]                  */
/*          int index: start point in arr[] to begin shifting        */
/*          int amount: amount of indices we are shifting them down  */
/*                                                                   */
/*      Description:                                                 */
/*          this shifts all the elements from index to max down      */
/*          by amount in arr[]                                       */
/*                                                                   */
/*********************************************************************/
int shift_strings_down( char* arr[], int* arr_cnt, int index, int amount )
{
    // check that index is within arr_cnt
    if( index >= *arr_cnt )
    {
        fprintf( stderr, "Error: shift_strings_down() - index (%d) > arr_cnt (%d)\n", index, *arr_cnt );
        return FAILURE;
    } 
    
    int i, j;

    // move all elements down by amount, starting at arr[index]
    for( i = index; i < *arr_cnt; i++ )
    {
        // free memory previously used
        free( arr[i - amount] );
    
        // place arr[index] in arr[index - amount]
        if( add_string( arr[i], arr, i - amount ) == FAILURE )
        {
            fprintf( stderr, "Error: shift_strings_down() - could not move cmds[%d] (%s) to cmds[%d]\n", i, arr[i], i - amount );
            return FAILURE;
        }
    }

    // free/NULL strings after arr_cnt - amount
    for( j = i - amount; j < i; j++ )
    {
        free( arr[j] );
        arr[j] = NULL;
    }

    // reset arr_cnt
    *arr_cnt -= amount; 

    return SUCCESS;
}/* end shift_strings_down() */


/*********************************************************************/
/*                                                                   */
/*                    HELPER FUNCTIONS (LOW LEVEL)                   */
/*                                                                   */
/*********************************************************************/


/*********************************************************************/
/*                                                                   */
/*      Function name: is_special_char                               */
/*      Return type:   int                                           */
/*      Parameter(s):                                                */
/*          const char c: character being tested                     */
/*                                                                   */
/*      Description:                                                 */
/*          tests a character against some special characters the    */
/*          parser should be able to detect                          */
/*                                                                   */
/*********************************************************************/
int is_special_char( const char c )
{
    return ( c == '$' || c == '|' || c == '<' || 
             c == '>' || c == '&' || c == '?' ||
             c == '!' || c == ',' || c == '=' || 
             c == ':'
           );
} /* end is_special_char() */



/*********************************************************************/
/*                                                                   */
/*      Function name: save_word                                     */
/*      Return type:   none                                          */
/*      Parameter(s):                                                */
/*          char* cmd: cmd we are adding                             */
/*          char* cmds[]: where we are adding cmd to                 */
/*          int* pos: position in cmds[] we are adding cmd           */
/*          int* cmd_len: len of cmd                                 */
/*                                                                   */
/*      Description:                                                 */
/*          used by parse_string function to save each word.         */
/*          calls the add_string() function which does the memory    */
/*          allocation and placement, this function more handles     */
/*          the checks and resets of variables.                      */
/*          It is done this way to modularize the code               */
/*                                                                   */
/*********************************************************************/
void save_word( char* cmd, char* cmds[], int* pos, int* cmd_len )
{
    // save word
    if( *cmd_len > 0 )
    {
        // add string to cmds[] and clear cmd
        if( add_string( cmd, cmds, (*pos)++ ) == FAILURE )
        {
            fprintf( stderr, "Error: could not save word - %s\n", cmd );
            return;
        }
    
        memset( &cmd[0], 0, WORD_LIMIT * sizeof(char) );
        *cmd_len = 0;
    }
    return;

} /* end is_special_char() */
