#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#define TRUE 1
#define FALSE 0

extern char **get_line();

/* Struct to remember which options were used.
 */
typedef struct {
    int set;
    int left_paren;
    int right_paren;
    int file_in;
    int file_out;
    int pipe;
    int background;
    int semi;
} specialflags;


/* Check to see if any single letter argument is a special
 * one.  If it is a special char, set its value to true and
 * also set the "set" variable to set, which is useful to see
 * if any special options were used.
 */
void special_char(char *special, specialflags *flags){
    char s = special[0];
    switch(s){
    case '(' : flags->set = flags->left_paren = TRUE; break;
    case ')' : flags->set = flags->right_paren = TRUE; break;
    case '<' : flags->set = flags->file_in = TRUE; break;
    case '>' : flags->set = flags->file_out = TRUE; break;
    case '|' : flags->set = flags->pipe = TRUE; break;
    case '&' : flags->set = flags->background = TRUE; break;
    case ';' : flags->set = flags->semi = TRUE; break;
    default  : break;
    }
}

/* Reset all special flags back to zero, so they don't carry
 * over to the next command.
 */
void reset_flags(specialflags *f){
    f->set = f->left_paren = f->right_paren = f->file_in =
            f->file_out = f->pipe = f->background = f->semi = FALSE;
}

void redirect_output (char **args, int length) {
    const char *path;
    path = args[length - 1];
    freopen (path, "w", stdout);
}

void redirect_input (char **child_argv){
    const char *path;
}

//potential function to goto when trapping ctrl c.
//lex is failing with ctrl-c input now, so this wont work anyway.
//-Chase
void sigproc()
{ 		 
    printf("you have pressed ctrl-c\n");
}

int main() {
    int i;
    char **args;
    char *prompt = "$ ";
    specialflags special_flags = {0,0,0,0,0,0,0};
    //signal(2, sigproc); //trap ctrl-c
    while(TRUE) {
        printf(prompt);
        args = get_line();

        char **child_argv = malloc (sizeof (char *));
        child_argv[0] = args[0];

        for(i = 0; args[i] != NULL; i++) {
            if(strlen (args[i]) == 1) {
                special_char (args[i], &special_flags);
            }

            printf("Argument %d: %s\n", i, args[i]);

            if(strcmp(args[i],"exit") == 0) return 0;
        }

        pid_t pid = fork();

        if (pid == 0){
            //child process
            if(special_flags.set == TRUE){
                if(special_flags.file_in == TRUE) redirect_input(child_argv);

                if (special_flags.file_out == TRUE) {\
                    redirect_output (args, i);
                }
            }

            if(special_flags.set && special_flags.background){
                freopen("/dev/null", "w", stdout); //redirect stdout and stderr
                freopen("/dev/null", "w", stderr);
            }

            if(execvp(args[0], child_argv) != 0)
                printf ("Command: %s not found\n",args[0]);
        } else {
            //parent
            int status;
            if(!special_flags.set || !special_flags.background) //dont wait if background
                waitpid (-1, &status, 0);
        }

        reset_flags(&special_flags);
    }
}
