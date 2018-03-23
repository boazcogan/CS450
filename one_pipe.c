#include <stdio.h>
#include "constants.h"
#include "parsetools.h"
#include <sys/wait.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<errno.h>
#include<string.h>

void execute_cmd(char ** line_words);
void syserror( const char * );


int main() {
    // Buffer for reading one line of input
    char line[MAX_LINE_CHARS];
    char* line_words[MAX_LINE_WORDS + 1];
    // Loop until user hits Ctrl-D (end of input)
    // or some other input error occurs
    while( fgets(line, MAX_LINE_CHARS, stdin) ) {
        int num_words = split_cmd_line(line, line_words);

        // Just for demonstration purposes
        for (int i=0; i < num_words; i++)
        {
            if (line_words[i] == "^D")
                break;
        }
        // Lets not worry about executing a command just yet...
        int last_pipe_index = 0;
        char** all_args[MAX_LINE_WORDS];
        int total_args = 0;
        // I want an array that looks like, [cmd1,cmd2,cmd3,cmd4] where each comma is a pipe.
        // So put all but the last command in an array called all args
        for (int i=0;i<num_words;i++)
	{
            if (*(line_words[i]) == '|')
            {
                 //char* whole_arg[MAX_LINE_WORDS +1];
                 char ** whole_arg = malloc(MAX_LINE_WORDS+1*sizeof(char*));
                 printf("pipe_encountered");
                 for (int j = last_pipe_index; j<i;j++)
                 {
                     whole_arg[j-last_pipe_index]=line_words[j];
                 }
                 whole_arg[i] = NULL;
                 all_args[total_args] = whole_arg;
                 total_args++;
                 last_pipe_index = i;
            }
        }
        // put the last command in there too
        //char* whole_arg[MAX_LINE_WORDS +1];
        char ** whole_arg = malloc(MAX_LINE_WORDS+1*sizeof(char*));
        for (int i = last_pipe_index+1; i < num_words; i++)
        {
            whole_arg[i-(last_pipe_index+1)] = line_words[i];
        }
        whole_arg[num_words-last_pipe_index] = NULL;
        all_args[total_args]=whole_arg;
        total_args++;
        // Now we can systematically handle each command individually
        //execute_cmd(line_words);
        // For piping there are cases, is it the only command (no pipes), is it
        // the last command, or is it an intermediate command.
        // Only command

        if (total_args == 1)
        {
            execute_cmd(all_args[0]);
        }
        // there is some sort of pipe or redirection occuring
        else
        {
            // handle the first pipe in the sequence out of the loop since it is unique
            int pfd[2];
            pid_t pid;
            if ( pipe (pfd) == -1 )
                syserror( "Could not create a pipe" );
            switch ( pid = fork() )
            {
                case -1:
                    syserror( "First fork failed" );
                case  0:
                    if ( close( 0 ) == -1 )
                        syserror( "Could not close stdin" );
                    dup(pfd[0]);
                    if ( close (pfd[0]) == -1 || close (pfd[1]) == -1 )
                       syserror( "Could not close pfds from first child" );
                   execvp(all_args[0][0], all_args[0]);
                   syserror( "Could not ls");
            
            }

            for (int i = 1; i < total_args; i++)
            {
                switch ( pid = fork() ) 
                {
                    case -1:
                       syserror( "Second fork failed" );
                    case  0:
                        if ( close( 1 ) == -1 )
                            syserror( "Could not close stdout" );
                        dup(pfd[1]);
                        if ( close (pfd[0]) == -1 || close (pfd[1]) == -1 )
                            syserror( "Could not close pfds from second child" );
                        execvp(all_args[i][0], all_args[i]);
                        syserror( "Could not exec wc" );
            
                }
            }
        }

        printf("\n\n\n");
    }

    return 0;
}


void execute_cmd(char ** line_words)
{
    int pid = fork();
    if ( pid == 0 )
    { 
        execvp(line_words[0], line_words);
    }    
}

void syserror(const char *s)
{
    extern int errno;

    fprintf( stderr, "%s\n", s );
    fprintf( stderr, " (%s)\n", strerror(errno) );
    exit( 1 );
}
