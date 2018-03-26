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
void multiple_cmds(char *** all_cmds, int num_cmds, int num_pipes);
void single_pipe(char *** all_cmds);


int main() {
    // Buffer for reading one line of input
    char line[MAX_LINE_CHARS];
    char* line_words[MAX_LINE_WORDS + 1];
    // Loop until user hits Ctrl-D (end of input)
    // or some other input error occurs
    while( fgets(line, MAX_LINE_CHARS, stdin) ) {
        int num_words = split_cmd_line(line, line_words);

        // Lets not worry about executing a command just yet...
        int last_pipe_index = 0;
        char** all_args[MAX_LINE_WORDS];
        int total_args = 0;
        // I want an array that looks like, [cmd1,cmd2,cmd3,cmd4, ..., cmd n] where each comma is a pipe.
        // So put all but the last command in an array called all args
        int total_pipes = 0;
        // keep track of two variables that will determine if redirection needs to occur
        int redirect_in = 0;
        int redirect_out = 0;
        char * redirect_in_from;
        char * redirect_out_from;
        
        for (int i=0;i<num_words;i++)
	{
            if (*(line_words[i])=='>')
            {
                // set the redirect in flag, store the place we're redirecting from, and move i past the current location.
                redirect_out = 1;
                redirect_out_from = line_words[i+1];
            }
            if (*(line_words[i])=='<')
            {
                // set the redirect in flag, store the place we're redirecting from, and move i past the current location.
                redirect_in = 1;
                redirect_in_from = line_words[i+1];
            }
            if (*(line_words[i]) == '|')
            {
                 char ** whole_arg = malloc(MAX_LINE_WORDS+1*sizeof(char*));
                 int pipe_encountered = 0;
                 for (int j = last_pipe_index; j<i;j++)
                 {

                     if (*(line_words[j]) == '|')
                         pipe_encountered ++;
                     else if (*(line_words[j]) == '<')
                         j+=1;
                     else if (*(line_words[j]) == '>')
                         j+=1;
                     else
                         whole_arg[j-last_pipe_index-pipe_encountered]=line_words[j];
                 }
                 total_pipes += 1;
                 whole_arg[i] = NULL;
                 all_args[total_args] = whole_arg;
                 total_args++;
                 last_pipe_index = i;
            }
        }
        // put the last command in there too
        char ** whole_arg = malloc((MAX_LINE_WORDS+1)*sizeof(char*));
        if (total_pipes == 0)
        {
             for (int i = last_pipe_index; i < num_words; i++)
             {
                 if (*(line_words[i]) == '<')
                     break;
                 else if (*(line_words[i]) == '>')
                     break;
                 whole_arg[i-(last_pipe_index)] = line_words[i];
             }
        }
        else
        {
            for (int i = last_pipe_index+1; i < num_words; i++)
            {
                 if (*(line_words[i]) == '<')
                     break;
                 else if (*(line_words[i]) == '>')
                     break;
                whole_arg[i-(last_pipe_index+1)] = line_words[i];
            }
        }
        whole_arg[num_words-last_pipe_index] = NULL;
        all_args[total_args]=whole_arg;
        total_args++;
        // Now we can systematically handle each command individually
        //execute_cmd(line_words);
        // For piping there are cases, is it the only command (no pipes), is it
        // the last command, or is it an intermediate command.
        // Only command
        printf("total pipes: %d", total_pipes); 

        if (total_args == 1)
        {
            execute_cmd(all_args[0]);
        }
        else
        {
            multiple_cmds(all_args, total_args, total_pipes);
        }
        // there is some sort of pipe or redirection occuring
        printf("\n");
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

void multiple_cmds(char *** all_cmds, int num_cmds, int num_pipes)
{
    // create all the pipes [pipe1, pipe2, pipe3, pipe4, ... , pipeNum_pipes]
    // note: pfd[1] = in, pfd[0] = out
    pid_t pid;
    int * pfds[num_pipes];
    for (int i = 0; i<num_pipes;i++)
    {
        int pfd = malloc(sizeof(int));
        pfds[i]=pfd;
        if (pipe(pfds[i]) == -1)
        {
            syserror( "Could not create pipes");
        }
    }
    // for every pipe im going to run two commands and reroute the out of one to
    // the in of another
    int current_pipe = 0;
    for (int i = 0; i < num_cmds; i++)
    {
        // if its the first command:
        if (i == 0)
        {// run it and put its out to the in of the pipe.
            switch ( pid = fork() )
            {
                case -1:
                    syserror( "First fork failed" );
                case  0:
                    if ( close( 1 ) == -1 )
                        syserror( "Could not close stdout" );
                    dup(pfds[current_pipe][1]);
                    for (int j = 0; j < num_pipes;j++)
                    {
                        if (close(pfds[j][0])==-1 || close(pfds[j][1]) == -1)
                        {
                            syserror("Could not close pfds from child");
                        }
                    }
                    execvp(all_cmds[i][0], all_cmds[i]);
                    syserror( "Could not run first command");
            
            }  
        }
        else if (i < num_cmds-1)
        {// run it and put its in to the out of the pipe, 
         // also put its out to the in of the next pipe.
         // lets start by moving onto the next pipe:
            current_pipe++;
            switch ( pid = fork() )
            {
                case -1:
                    syserror( "intermediate fork failed" );
                case  0:
                    if ( close( 0 ) == -1)
                        syserror( "Could not close stdin" );
                    dup(pfds[current_pipe-1][0]);
                    if ( close( 1 ) == -1)
                        syserror( "Could not close stout" );
                    dup(pfds[current_pipe][1]);
                    for (int j = 0; j < num_pipes;j++)
                    {
                        if (close(pfds[j][0])==-1 || close(pfds[j][1]) == -1)
                        {
                            syserror("Could not close pfds from child");
                        }
                    }
                    execvp(all_cmds[i][0], all_cmds[i]);
                    syserror( "Could not run intermediate command");


            }

        }
        else
        {
        // its the last pipe so we handle it differently
        // move on with the pipe
            current_pipe++;
            switch ( pid = fork() )
            {
                case -1:
                    syserror( "last fork failed" );
                case  0:
                    if ( close( 0 ) == -1)
                        syserror( "Could not close stdin" );
                    dup(pfds[current_pipe-1][0]);
                    for (int j = 0; j < num_pipes;j++)
                    {
                        if (close(pfds[j][0])==-1 || close(pfds[j][1]) == -1)
                        {
                            syserror("Could not close pfds from child");
                        }
                    }
                    execvp(all_cmds[i][0], all_cmds[i]);
                    syserror( "Could not run last command"); 
            }
         } 
    }
    for (int j = 0; j < num_pipes;j++)
    {
        if (close(pfds[j][0])==-1 || close(pfds[j][1]) == -1)
        {
            syserror("Could not close pfds from parent");
        }
    }
    while(wait(NULL) != -1);
}

