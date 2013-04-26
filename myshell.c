/*
 * CREATED: Chase Wilson    April 10, 2013
 * CHANGED: Nicholas Smith  April 10, 2013
 * CHANGED: Chase Wilson    April 15, 2013
 * CHANGED: Nicholas Smith  April 15, 2013
 * CHANGED: Maria Mishkova  April 16, 2013
 * CHANGED: Paul Scherer    April 16, 2013
 * CHANGED: Nicholas Smith  April 17, 2013
 * CHANGED: Chase Wilson    April 17, 2013
 * CHANGED: Paul Scherer    April 17, 2013
 * CHANGED: Paul Scherer    April 18, 2013
 * CHANGED: Chase Wilson    April 24, 2013
 */

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define TRUE 1
#define FALSE 0

extern char **get_line();
int error;
int free_args = FALSE;
char **savedargs = NULL;
int saved_pid = 0;

/* Struct to remember which options were used.
 */
typedef struct {
    int left_paren;
    int right_paren;
    int file_in;
    int file_out;
    int pipe;
    int background;
    int semi;
} specialflags;

typedef struct {
    char *path_in;
    char *path_out;
    char *path_pipe;
} specialpaths;

void print_error (char *string) {
    error = TRUE;
    fprintf (stderr,"myshell: %s\n",string);
}

/* Set the path variables if command requests it
 */
void set_path (int *i,char **args, char **path) {
    if (args[*i+1] != NULL)
        *path = args[++(*i)];
    else
        print_error ("syntax error");
}

/* if you see a semicolon, then it removes the rest of the args
 * from args, and saves them into 'savedargs' so that they can
 * be handled later.
 * -Chase
 */
void separate_args (int *i, char **args) {
    int j = *i + 1;
    int count = 0;

    for (; args[j] != NULL; j++) {
        count++;
    }

    //+1 for null terminator
    args[(*i)++] = NULL;

    savedargs = calloc (j + 1, sizeof (char *));
    assert (savedargs != NULL);
    int k;

    for (k = 0; k < count; k++) {
        savedargs[k] = args[*i];
        printf ("%s\n", savedargs[k]);
        args[(*i)++] = NULL;
    }
    printf ("Done\n");

    //add null terminator
    savedargs[k] = NULL;
    printf ("NULL\n");
}

/* Check to see if any single letter argument is a special
 * one. If it is a special char, set its value to true and
 * also set the "set" variable to set, which is useful to see
 * if any special options were used.
 */
int special_char (int *i, char **args, specialflags *flags,
        specialpaths *paths) {
    switch (args[*i][0]){
        case '(' : flags->left_paren = TRUE; break;
        case ')' : flags->right_paren = TRUE; break;
        case '<' : flags->file_in = TRUE;
                   set_path(i, args, &paths->path_in); break;
        case '>' : flags->file_out = TRUE;
                   set_path(i, args, &paths->path_out); break;
        case '|' : flags->pipe = TRUE;
                   set_path(i, args, &paths->path_pipe); break;
        case '&' : flags->background = TRUE; break;
        case ';' : flags->semi = TRUE;
                   separate_args(i, args); break;
        default : return FALSE;
    }

    return TRUE;
}

/* Reset all special flags back to zero, so they don't carry
 * over to the next command.
 */
void reset_flags (specialflags *f) {
    f->left_paren = f->right_paren = f->file_in =
            f->file_out = f->pipe = f->background = f->semi = FALSE;
}

void parse_args (specialflags *special_flags, specialpaths *special_paths,
        char **args, char **child_args) {
    int i,j;

    for (i = j = 0; args[i] != NULL; i++) {
        printf ("strlen\n");
        if (strlen (args[i]) == 1 && special_char (&i, args,
                special_flags, special_paths))
            continue;
        child_args[j++] = args[i];
        printf ("parse_args: %s\n", child_args[j - 1]);
    }
    printf ("parse_args: Done\n");

    child_args[j] = NULL;
}

void execute (specialflags *special_flags, specialpaths *special_paths,
        char **child_args) {
    if (strcmp (child_args[0],"cd") == 0) {
        chdir((child_args[1] == NULL) ? getenv("HOME") : child_args[1]);
    } else {
        pid_t pid = fork();

        if (pid == 0) {

            // Child process
            if (special_flags->background)
                setpgid (0, 0);

            /*
             * Creates a file from the path provided if file not
             * found. Redirects the output to the file specified.
             */
            if (special_flags->file_out) {
                freopen (special_paths->path_out, "w", stdout);
            }

            if (special_flags->file_in) {
                freopen (special_paths->path_in,"r",stdin);
            }

            if(execvp (child_args[0], child_args) != 0) {
                printf ("Command: %s not found\n", child_args[0]);
                exit(1);
            }
        } else {

            // Parent
            int status;
            saved_pid = pid;

            // Don't wait if background
            if (!special_flags->background)
                waitpid (-1, &status, 0);
            else {
                printf ("Process %d in background.\n", (int) pid);
            }
        }
    }
}

int shell_cmd (char **args){
    if (args[0] == NULL) return 1;

    if (strcmp (args[0],"exit") == 0) exit(0);

    /*if (strcmp (args[0],"cd") == 0) {
        chdir((args[1] == NULL) ? getenv("HOME") : args[1]);
        return 1;
    }*/

    return 0;
}

//called when a child process is exited.
//-chase
void sig_handler() {
    int status;

    waitpid(-1, &status, WNOHANG);
}

void kill_sig() {

    //catch ctrl+c
    if (saved_pid != 0) {
        kill (saved_pid,SIGKILL);
        saved_pid = 0;
    }
}

int exec_shell () {
    char **args;
    char *prompt = "\n$ ";
    specialflags special_flags = {0,0,0,0,0,0,0};
    specialpaths special_paths = {NULL,NULL,NULL};
    error = FALSE;

    while (TRUE) {
        if (savedargs == NULL) {
            printf (prompt);
            args = get_line();
        } else {
            args = malloc (sizeof (savedargs) * sizeof (char *));
            assert (args != NULL);
            int i;

            for(i = 0; savedargs[i] != NULL; i++) {
                args[i] = savedargs[i];
                printf ("%s\n", args[i]);
            }

            //add terminator
            args[i] = NULL;
            free (savedargs);
            savedargs = NULL;
            free_args = TRUE;
        }

        if (shell_cmd (args)) continue;

        char **child_args = malloc (sizeof (args) * sizeof (char *));
        assert (child_args != NULL);

        printf ("Parsing args\n");
        parse_args (&special_flags, &special_paths, args, child_args);
        printf ("Done parsing\n");

        if (special_flags.left_paren) {
            savedargs = args;
            exec_shell ();
        } else {
            printf ("Executing\n");
            execute (&special_flags, &special_paths, child_args);
            reset_flags (&special_flags);
            error = FALSE;

            if (free_args) {
                free (args);
                free_args = FALSE;
            }

            free (child_args);
        }
    }
}

int main() {
    signal (SIGINT, kill_sig);
    signal (SIGCHLD, sig_handler);

    return exec_shell ();
}
