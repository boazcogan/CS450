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
char * break_down_args(char ** line_words, int numwords);


int main() {
    // Buffer for reading one line of input
    char line[MAX_LINE_CHARS];
    char* line_words[MAX_LINE_WORDS + 1];
    // Loop until user hits Ctrl-D (end of input)
    // or some other input error occurs
    while( fgets(line, MAX_LINE_CHARS, stdin) ) {
        int num_words = split_cmd_line(line, line_words);
        // Just for demonstration purposes
        char * broken_down_args = break_down_args(line_words, num_words); 
        char * first = broken_down_args[0];
        char * second = broken_down_args[1];
        if (second == NULL)
            execute_cmd(line_words);
        else
            printf("else");

        
        
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

char * break_down_args(char ** line_words, int numwords)
{
        char* cmds[MAX_LINE_CHARS];
        // store the idx of the last encountered pipe
        int last_pipe = 0;
        int cmds_counter = 0;

        for (int i=0; i < numwords; i++)
        {
            if (line_words[i] == "^D")
                break;
            // Break the arg into pieces, cmd1 | cmd2 | cmd3 s.t.
            // char array is [cmd1, cmd2, cmd3]
            //
            // So, if a pipe is encountered:
            if (line_words[i] == "|")
            {
                // store up until i in a char array.
                char * current_command[20];
                int counter = 0;
                // loop from the last encountered pipe to the current position
                for (int j = last_pipe; j < i; j++)
                {
                    current_command[counter] = line_words[j];
                    counter++;
                }
                cmds[cmds_counter] = current_command;
                cmds_counter++;
                last_pipe = i+1;
            }

        }
        if (last_pipe == 0)
        {
           cmds[0] = line_words;
           return cmds;
        }
        else
        {
           for (int i=0;i<cmds_counter;i++)
           {
               printf("the current command is: %s\n", cmds[i][0]);
           }
           return cmds;
        }
}





