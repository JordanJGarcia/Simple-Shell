#include "execution_module.h"

/*********************************************************************/
/*                                                                   */
/*      Function name: execute                                       */
/*      Return type:   void                                          */
/*      Parameter(s):  None                                          */
/*                                                                   */
/*      Description:                                                 */
/*          executes a program entered in the command line by user.  */
/*                                                                   */
/*********************************************************************/
void execute( int infile_pos, int outfile_pos, int n_pipes, int pipe_loc[] )
{
    int pid, fd_in, fd_out, pos_to_null = -1; 

    // attempt to open input file
    if( infile_pos != -1 )
        fd_in = open( cmds[infile_pos], O_RDONLY );
    else
        fd_in = STDIN_FILENO;

    // error handling for input file
    if( fd_in == -1 )
    {
        fprintf( stderr, "Error: cannot open input file %s\n", cmds[infile_pos] );
        return;
    }
    
    // attempt to open output file
    if( outfile_pos != -1 )
        fd_out = open( cmds[outfile_pos], O_RDWR | O_CREAT, 0666 );
    else
        fd_out = STDOUT_FILENO;

    // error handling for output file
    if( fd_out == -1 )
    {
        fprintf(stderr, "Error: can't open output file %s\n", cmds[outfile_pos] );
        return;
    }

    // set NULL terminator for execvp()
    // we can NULL each cmd in cmds[] that is a redirection operator ('<','>')
    if( infile_pos != -1 )
    {
        free( cmds[infile_pos - 1] );
        cmds[infile_pos - 1] = NULL;
    }

    if( outfile_pos != -1 )
    {
        free( cmds[outfile_pos - 1] );
        cmds[outfile_pos - 1] = NULL;
    }
    
    // handle pipelines if needed, or spawn process and execute prog 
    if( n_pipes > 0 )
        execute_and_pipe( n_pipes, pipe_loc, fd_in, fd_out );
    else
        pid = generate_process( fd_in, fd_out, cmds );

    return;

}/* end execute() */



/*********************************************************************/
/*                                                                   */
/*      Function name: execute_and_pipe                              */
/*      Return type:   void                                          */
/*      Parameter(s):                                                */
/*          int n_pipes: number of pipes entered in command line.    */
/*                                                                   */
/*      Description:                                                 */
/*          executes a program entered in the command line by user   */
/*          and creates pipelines through the programs.              */
/*                                                                   */
/*********************************************************************/
static void execute_and_pipe( int n_pipes, int pipe_loc[], int fd_in, int fd_out )
{
    char** current_cmd = cmds;
    int i, j, pipe_fd[n_pipes][2];
    pid_t pid; 

    // create first pipeline
    if ( pipe( pipe_fd[0] ) == -1 )
    {
        fprintf( stderr, "Error: Calling pipe() failed.\n" );
        return;
    } // pipe has been created 

    /* process programs */
    for ( i = 0; i < n_pipes; i++ )
    {
        // set index of pipe to null so execvp knows where to stop 
        free( cmds[pipe_loc[i]] );
        cmds[pipe_loc[i]] = NULL;

        // first cmd running  
        if ( i == 0 )
        {
            // execute command, this process redirects output to write end of current pipe
            pid = generate_process( fd_in, pipe_fd[i][WRITE_END], current_cmd );
        }
        else
        {
            // create another pipeline
            if ( pipe( pipe_fd[i] ) == -1 )
            {
                fprintf( stderr, "Error: Calling pipe() failed.\n" );
                
                // close all pipes we've created
                for( j = 0; j < i; j++ )
                {
                    close( pipe_fd[j][READ_END] );
                    close( pipe_fd[j][WRITE_END] );
                }
                return;
            } // pipe has been created

            // execute command, this time the process redirects input from read end of previous pipe
            // and redirects output to write end of current pipe
            pid = generate_process( pipe_fd[i-1][READ_END], pipe_fd[i][WRITE_END], current_cmd );
        }

        // adjust current_cmd to point to next set of cmds 
        current_cmd = &cmds[pipe_loc[i] + 1];
    }

    // run final command, this will write to stdout and read from read end of previous pipe
    pid = generate_process( pipe_fd[i-1][READ_END], fd_out, current_cmd );

    // close all pipes we created
    for ( i = 0 ; i < n_pipes; i++ )
    {
        close( pipe_fd[i][READ_END] );
        close( pipe_fd[i][WRITE_END] );
    }

    return;
} /* end execute_and_pipe */



/*********************************************************************/
/*                                                                   */
/*      Function name: generate_process                              */
/*      Return type:   void                                          */
/*      Parameter(s):  None                                          */
/*                                                                   */
/*      Description:                                                 */
/*          creates a process and executes a program.                */
/*                                                                   */
/*********************************************************************/
static int generate_process( int fd_in, int fd_out, char* prog[] )
{
    pid_t pid, pgid = getpgrp();
    int status, w;
    void (*istat)(int), (*qstat)(int);

    // if in child process 
    if( ( pid = fork() ) == 0 )
    {
        // if we are not directing to stdout, reassign output
        if ( fd_out != STDOUT_FILENO )
        {
            dup2( fd_out, STDOUT_FILENO );
            close( fd_out );
        }

        // if we are not getting from stdin, reassign input 
        if ( fd_in != STDIN_FILENO )
        {
            dup2( fd_in, STDIN_FILENO );
            close( fd_in );
        }

        // execute command
        // in the event of the command execution failing, we need to exit(1)
        // this is because execvp essentially clones the parent process into a 
        // new shell to run the program, and if we don't exit on failure,
        // we will need to "exit" the shell as many times as child processes created.
        if( execvp( prog[0], prog ) == -1 )
        {
            fprintf( stderr, "Error: cannot run the program '%s'\n", prog[0] );
            fprintf( stderr, "       this may be because the program is not in $PATH or\n" );
            fprintf( stderr, "       there is not an alias specified for this command, among other possibilities\n" );
            exit(1);
        }
    } // parent process 

    // set process group ID if it does not match process ID
    if( pid != pgid )
        setpgid( pid, pgid );

    // ignore ctrl-c & ctrl-\ while we wait for child process to finish
    // this is so if we try to quit a program we are running (child process)
    // we don't completely exit the shell
    istat = signal(SIGINT, SIG_IGN);
    qstat = signal(SIGQUIT, SIG_IGN);

    // close descriptors if necessary in parent 
    if ( fd_in != STDIN_FILENO )
        close( fd_in );

    if ( fd_out != STDOUT_FILENO )
        close( fd_out );

    // wait for child processes to finish
    while( ( w = wait( &status ) ) != pid && w != -1 )
        continue;

    // allow for ctrl-c & ctrl-\ now that we are done with child process
    signal(SIGINT, istat);
    signal(SIGQUIT, qstat);

    return pid;
} /* end generate_process */
