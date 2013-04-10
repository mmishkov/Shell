#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#define TRUE 1
#define FALSE 0

extern char **get_line();

main() {
  int i;
  char **args; 

  while(TRUE) {
    args = get_line();
    for(i = 0; args[i] != NULL; i++) {
      pid_t pid = fork();
      if(pid==0)
      printf("Argument %d: %s\n", i, args[i]);
    }
  }
}
