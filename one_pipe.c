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
                 char* whole_arg[MAX_LINE_WORDS +1];
                 printf("pipe_encountered");
                 for (int j = last_pipe_index; j<i;j++)
                 {
                     whole_arg[j-last_pipe_index]=line_words[j];
                 }
                 whole_arg[i] = NULL;
                 all_args[total_args] = whole_arg;
                 total_args++;
            }
        }
        // put the last command in there too
        char* whole_arg[MAX_LINE_WORDS +1];
        for (int i = last_pipe_index; i < num_words; i++)
        {
            whole_arg[i-last_pipe_index] = line_words[i];
        }
        all_args[total_args]=whole_arg;
        // Now we can systematically handle each command individually
        //execute_cmd(line_words);
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



