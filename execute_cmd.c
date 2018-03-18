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

        printf("all of the line is: %s\n\n", line);
        // Just for demonstration purposes
        for (int i=0; i < num_words; i++)
        {
            printf("line_words at i is: %s\n\n", line_words[i]);
            printf("i is currently: %d\n\n", i);
            if (line_words[i] == "^D")
                break;
            printf("\nThe line is: %s\n", line);
        }
        printf("before execvp, line_words[0] = %s\n\n", line_words[0]);
        execute_cmd(line_words);
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
