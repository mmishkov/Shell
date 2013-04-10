#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#define TRUE 1
#define FALSE 0

extern char **get_line();

enum {
   LPAREN,
   RPAREN,
   FIN,
   FOUT,
   PIPE,
   BKGRND
};

typedef struct {
   int left_paren;
   int right_paren;
   int file_in;
   int file_out;
   int pipe;
   int background;
} specialflags;

void special_char(char *special, specialflags *flags){
   char s = special[0];
   switch(s){
      case '(' : flags->left_paren =1; break;
      case ')' : flags->right_paren = 1; break;
      case '<' : flags->file_in = 1; break;
      case '>' : flags->file_out = 1; break;
      case '|' : flags->pipe = 1; break;
      case '&' : flags->background = 1; break;
      default  : break;
   }
}

main() {
  int i;
  char **args; 
  specialflags special_flags = {0,0,0,0,0,0};

  while(TRUE) {
    args = get_line();
    for(i = 0; args[i] != NULL; i++) {
      if(strlen(args[i])==1) special_char(args[i],&special_flags);
      printf("Argument %d: %s\n", i, args[i]);
    }
    char *child_argv[] = {"ls", NULL};
    pid_t pid = fork();
    if(pid==0){
       if(execvp(args[0],child_argv)!=0)
          printf("Command: %s not found\n",args[0]);
    }else{
    }
  }
}
