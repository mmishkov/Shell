include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>

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

void redirect_input(char **child_argv){
        const char *path;
}
        
/*potential function to goto when trapping ctrl c.
//lex is failing with ctrl-c input now, so this wont work anyway.
//-Chase*/
void sigproc()
{
        printf("you have pressed ctrl-c\n");
}

char execute( char **args)
{
int status,n;
 pid_t pid;
 char **child_argv = args; 
 specialflags special_flags = {0,0,0,0,0,0,0};

 pid = fork();
                
  if (pid  < 0)
  {     /* fork a child process           */
       printf("*** ERROR: forking child process failed\n");
       exit(1);
   }
   else if (pid == 0)
   { /* for the child process */

                        if(special_flags.set == TRUE)
                        {
                                if(special_flags.file_in==TRUE) redirect_input(child_argv);
                        }
                        if(special_flags.set && special_flags.background)
                        {
                                freopen("/dev/null", "w", stdout); /*redirect stdout and stderr*/
                                freopen("/dev/null", "w", stderr);
      }
                        if(execvp(args[0],child_argv) < 0)
                        {  
                                printf("Command: %s not found\n",args[0]);
                                exit(1);
                         }
        }       
        else   
        { /*for the parent*/
                        if(!special_flags.set || !special_flags.background) //dont wait if background
                        //while (wait(&status) != pid)       /* wait for completion  */
                         //;
                                (waitpid(-1,&status,0));
   
        }
                        
                return status;
                                
}
                        
main()
{

         int i;
         char **args;
        char *prompt = "$ ";
        specialflags special_flags = {0,0,0,0,0,0,0};
        /*signal(2, sigproc); //trap ctrl-c*/
        while(TRUE)
        {       
                printf(prompt);
                args = get_line();
      if(strcmp(args[i],"exit") == 0) exit(0);
                        
                char **child_argv = args;
                for(i = 0; args[i] != NULL; i++)
                {
                        if(strlen(args[i])==1) special_char(args[i],&special_flags);
                        if(special_flags.set)
                        {
                                child_argv[i] = ""; //NULL was causing crash.
                        }
                        else
                        {
}
                        printf("Argument %d: %s\n", i, args[i]);
        
        
  }
        
        
         execute(args);
         reset_flags(&special_flags);
}
  }
