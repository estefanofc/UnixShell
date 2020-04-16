#include <stdio.h>
#include <unistd.h>
#include <assert.h>     // assert
#include <fcntl.h>      // O_RDWR, O_CREAT
#include <stdbool.h>    // bool
#include <stdio.h>      // printf, getline
#include <stdlib.h>     // calloc
#include <string.h>     // strcmp
#include <unistd.h>     // execvp
#include <sys/wait.h>   // wait
#define MAX_LINE 80 /* The maximum length command*/

int readline(char **buffer) {
  size_t len;
  int number_of_chars = getline(buffer, &len, stdin);
  if (number_of_chars > 0) {
    //get rid of \n
    (*buffer)[number_of_chars - 1] = '\0';
  }
  return number_of_chars;
}
int tokenize(char *line, char **tokens) {
  char *pch;

  pch = strtok(line, " ");
  int num = 0;
  while (pch != NULL) {
    tokens[num] = pch;
    num++;
    pch = strtok(NULL, " ");
  }
  return num;
}
int main() {
  char *args[MAX_LINE / 2 + 1];/* command line arguments */
  int should_run = 1; /* flag to determine when to exit program */
  char *command = (char *) malloc(MAX_LINE * sizeof(char));
  while (should_run) {
    printf("osh> ");
    fflush(stdout);
    int len = readline(&command);
    if (len <= 0)
      break;
    if (strcmp(command, "") == 0)
      continue;
    if (strcmp(command, "exit") == 0)
      break;
    // clear out args
    for (int i = 0; i < MAX_LINE / 2 + 1; ++i)
      args[i] = NULL;
    int num_of_tokens = tokenize(command, args);
    for (int i = 0; i < num_of_tokens; ++i)
      printf("%d. %s\n", i, args[i]);
    char **firstcmd = (char **) malloc(MAX_LINE * sizeof(char *));
    for (int i = 0; i <= num_of_tokens; ++i)
      firstcmd[i] = NULL;
    // up to first ";" or "&"
    int i = 0;
    while (args[i] != NULL && strcmp(args[i], ";") != 0) {
      firstcmd[i] = args[i];
      ++i;
    }
    execvp(*firstcmd, firstcmd);
    /**
    * After reading user input, the steps are:
    * (1) fork a child process using fork()
    * (2) the child process will invoke execvp()
    * (3) parent will invoke wait() unless command included &
    */
  }
  printf("Exiting shell\n");
  return 0;
}
