#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define TRUE 1
#define FALSE 0

extern char **get_line();

int error;
int free_args = FALSE;
char **savedargs = NULL; //used to save args

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

void print_error(char *string){
	error = TRUE;
	fprintf(stderr,"myshell: %s\n",string);
}

/* Set the path variables if command requests it
*/
void set_path(int *i,char **args, char **path){
	if(args[*i+1] != NULL) *path = args[++(*i)];
	else print_error("syntax error");
}

/* if you see a semicolon, then it removes the rest of the args
* from args, and saves them into 'savedargs' so that they can 
* be handled later.
* -Chase
*/
void separate_args(int *i,char **args){
	int j = *i + 1;
	int count = 0;
	while(args[j] != NULL){
		j+=1;
		count +=1;
	}
	args[*i] = NULL;
	*i+=1;
	savedargs = calloc(j+1,sizeof(char *));//+1 for null terminator
	int k = 0;
	for(k =0; k < count; k+=1){
		savedargs[k] = args[*i];
		args[*i] = NULL;
		*i+=1;
	}
	savedargs[k] = NULL; //add null terminator

}

/* Check to see if any single letter argument is a special
* one. If it is a special char, set its value to true and
* also set the "set" variable to set, which is useful to see
* if any special options were used.
*/
int special_char (int *i, char **args, specialflags *flags, specialpaths *paths) {
	switch(args[*i][0]){
	case '(' : flags->left_paren = TRUE; break;
	case ')' : flags->right_paren = TRUE; break;
	case '<' : flags->file_in = TRUE; 
		set_path(i,args,&paths->path_in); break;
	case '>' : flags->file_out = TRUE; 
		set_path(i,args,&paths->path_out); break;
	case '|' : flags->pipe = TRUE; 
		set_path(i,args,&paths->path_pipe); break;
	case '&' : flags->background = TRUE; break;
	case ';' : flags->semi = TRUE; separate_args(i,args); break;
	default : return FALSE;
	}
	return TRUE;
}



/* Reset all special flags back to zero, so they don't carry
* over to the next command.
*/
void reset_flags (specialflags *f){
	f->left_paren = f->right_paren = f->file_in =
		f->file_out = f->pipe = f->background = f->semi = FALSE;
}

char** parse_args(specialflags *special_flags,specialpaths *special_paths, 
	char **args){
		int args_size = 0;
		int keep_args_size = 0;
		int i,j;
		for(i = 0; args[i] != NULL; ++i) {
			args_size +=1;
			//printf("argument %d =%s\n",i,args[i]);
			if(strlen (args[i]) == 1)
				if(special_char(&i,args,special_flags,special_paths)) continue;
			keep_args_size+=1;
		}
		char **child_args = calloc(keep_args_size+1,sizeof(char *));
		assert (child_args != NULL);
		j = 0;
		for(i =0;i<keep_args_size;i+=1){
			if(args[i] != NULL){
				child_args[j] = args[i];
				j+=1;
			}
		}
		return child_args;
}

void execute(specialflags *special_flags,specialpaths *special_paths, 
	char **child_args){
		pid_t pid = fork();
		if (pid == 0) {
			// Child process
			/*
			* Creates a file from the path provided if file not
			* found. Redirects the output to the file specified.
			*/
			if (special_flags->file_out) {
				freopen(special_paths->path_out, "w", stdout);
			}
			if (special_flags->file_in){
				printf("stdin\n");
				freopen(special_paths->path_in,"r",stdin);
			}
			if(special_flags->background) {
				// Redirect stdout and stderr
				freopen ("/dev/null", "w", stdout);
				freopen ("/dev/null", "w", stdin);
				freopen ("/dev/null", "w", stderr);
			}
			if(execvp (child_args[0], child_args) != 0){
				printf ("Command: %s not found\n", child_args[0]);
				exit(1);
			}
		} else {
			// Parent
			int status;
			// Don't wait if background
			if (!special_flags->background)
				waitpid (-1, &status, 0);
		}
}

int shell_cmd(char **args){
	if(strcmp (args[0],"exit") == 0) exit(0);
	if(strcmp (args[0],"cd") == 0) {
		chdir((args[1] == NULL) ? getenv("HOME") : args[1]);
		return 1;
	}
	return 0;
}

//potential function to goto when trapping ctrl c.
//lex is failing with ctrl-c input now, so this wont work anyway.
//-Chase
void sigproc()
{
	printf ("you have pressed ctrl-c\n");
}

//called when a child process is exited.
//-chase
void sig_handler(int signal) {
	int status;
	int result = wait(&status);
}

int main() {
	char **args;
	char *prompt = "$ ";
	specialflags special_flags = {0,0,0,0,0,0,0};
	specialpaths special_paths = {NULL,NULL,NULL};
	error = FALSE;
	//signal(SIGCHLD, sig_handler); //listen for child exiting.
	//signal(2, sigproc); //trap ctrl-c
	/*      
	This isn't working in minix: error: 'sighandler_t'undeclared (first use in this function)
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
	signal(SIGINT, (sighandler_t) fprintf (stderr,"\n"));
	*/
	while(TRUE) {
		if(savedargs != NULL){
			args = malloc(sizeof(savedargs) * sizeof(char *));
			int i;
			for(i =0; savedargs[i] != NULL; i+=1){
				args[i] = savedargs[i];
			}
			args[i] = NULL; //add terminator
			free(savedargs);
			savedargs = NULL;
			free_args = TRUE;
		}
		else{
			printf (prompt);
			args = get_line();
		}
		if(shell_cmd(args)) continue;
		char **child_args= parse_args(&special_flags,&special_paths,args);
		execute(&special_flags,&special_paths,child_args);
		reset_flags (&special_flags);
		error = FALSE;
		if(free_args){
			free(child_args);
			free_args = FALSE;
		}
		//free(args);
	}
}
