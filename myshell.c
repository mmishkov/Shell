#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
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
void special_char (char *special, specialflags *flags) {
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
void reset_flags (specialflags *f){
    f->set = f->left_paren = f->right_paren = f->file_in =
            f->file_out = f->pipe = f->background = f->semi = FALSE;
}

//potential function to goto when trapping ctrl c.
//lex is failing with ctrl-c input now, so this wont work anyway.
//-Chase
void sigproc()
{
    printf ("you have pressed ctrl-c\n");
}

int main() {
    size_t i;
    size_t j;
    char **args;
    const char *path;
    char *prompt = "$ ";
    specialflags special_flags = {0,0,0,0,0,0,0,0};

    //signal(2, sigproc); //trap ctrl-c
    while(TRUE) {
        printf (prompt);

        args = get_line();

        /*
         * If the user passed "exit", exit the shell.
         */
        if(strcmp (args[0],"exit") == 0) return 0;

        /*
         * Calculate the amount of arguments passed and create a copy
         * of the arguments.
        */
        size_t args_length = sizeof (args);
        char **child_argv = malloc (args_length * sizeof (char *));
        assert (child_argv != NULL);

        for (i = 0; i < args_length; i++) {
            child_argv[i] = args[i];
        }

        /*
         * Go through each of the arguments passed. If a special
         * character is found, set a special flag indicating which
         * character.
         */
        for(i = 0; args[i] != NULL; i++) {
            if(strlen (args[i]) == 1) {
                special_char (args[i], &special_flags);

                /*
                 * If the special character is a ">", store the path
                 * passed, and move the arguments to the left by one.
                 * This allows the shell to perform the I/O redirection
                 * (execvp() won't perform redirection).
                 */
                if (strcmp (args[i], ">") == 0) {
                    path = args[i + 1];
                    for (j = i; j < args_length; j++) {
                        child_argv[j] = args[j + 2];
                    }
                }
            }

            printf ("Argument %d: %s\n", i, args[i]);
        }

        pid_t pid = fork();

        if (pid == 0) {
            // Child process
            if (special_flags.set == TRUE) {
                /*
                 * Creates a file from the path provided if file not
                 * found. Redirects the output to the file specified.
                 */
                if (special_flags.file_out == TRUE) {
                    freopen (path, "w", stdout);
                }
            }

            if(special_flags.set && special_flags.background) {
                // Redirect stdout and stderr
                freopen ("/dev/null", "w", stdout);
                freopen ("/dev/null", "w", stderr);
            }

            if(execvp (args[0], child_argv) != 0)
                printf ("Command: %s not found\n", args[0]);
        } else {
            // Parent
            int status;
            // Don't wait if background
            if (!special_flags.set || !special_flags.background)
                waitpid (-1, &status, 0);

        }

        reset_flags (&special_flags);
    }
}
