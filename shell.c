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
#define BUF_SIZE 128
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
  enum { READ, WRITE };
  char *args[MAX_LINE / 2 + 1];/* command line arguments */
  int should_run = 1; /* flag to determine when to exit program */
  int should_wait = 1;
  char *cmdLine = (char *) malloc(MAX_LINE * sizeof(char));
  pid_t pid;
  int pipeFD[2];
  //create pipe
  if (pipe(pipeFD) < 0) {
    perror("Error in creating pipe");
    exit(EXIT_FAILURE);
  }
  while (should_run) {
    printf("osh> ");
    fflush(stdout);
    int len = readline(&cmdLine);
    if (len <= 0)
      break;
    if (strcmp(cmdLine, "") == 0)
      continue;
    if (strcmp(cmdLine, "exit") == 0)
      break;
    // clear out args
    for (int i = 0; i < MAX_LINE / 2 + 1; ++i)
      args[i] = NULL;
    int num_of_tokens = tokenize(cmdLine, args);
    char **currCmd = (char **) malloc(MAX_LINE * sizeof(char *));
    for (int i = 0; i <= num_of_tokens; ++i)
      currCmd[i] = NULL;
    // up to first ";" or "&"
    int i = 0;
    while (args[i] != NULL && strcmp(args[i], ";") != 0) {
      if (strcmp(args[i], "&") == 0) {
        should_wait = 0;
        break;
      }
      currCmd[i] = args[i];
      i++;
    }
    int status;
    pid = fork();
    if(pid < 0) {
      perror("Error during fork");
      exit(EXIT_FAILURE);
    }
    if(pid == 0) {
      //Child
      close(pipeFD[READ]);
      dup2(pipeFD[WRITE], 1);
      //stdout is now child's read pipe
      execlp("/bin/cat", "cat", "pipeexeccat.cpp", NULL);
      if (pipeFD[0] != -1) {
        if (dup2(pipeFD[0], STDIN_FILENO) != STDIN_FILENO) {
          perror("dup2");
          exit(1);
        }
      }
      execvp(*currCmd, currCmd);
      perror("execvp");
      exit(0);
    } else {
      // Parent
      int status;
      int pid_completed = wait(&status);
      char buf[BUF_SIZE];
      int n = read(pipeFD[READ], buf, BUF_SIZE);
      buf[n] = '\0';
      for (int i = 0; buf[i] != '\0'; ++i) {
        printf(buf[i]);
      }
      close(pipeFD[0]);
      close(pipeFD[1]);
      if (should_wait)
        waitpid(pid, &status, 0);
      break;
    }
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
