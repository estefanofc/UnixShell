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

void redirect(char **cmd, bool input) {

}

void pipedCmd(char **cmd) {
  // 0 is read end, 1 is write end
  enum { READ, WRITE };
  int pipeFD[2];
  pid_t p1, p2;
  char **left = (char **) malloc(MAX_LINE * sizeof(char *));
  char **right = (char **) malloc(MAX_LINE * sizeof(char *));
  int i = 0;
  int j = 0;
  while (cmd[i] != NULL) {
    left[i] = NULL;
    right[i] = NULL;
    i++;
  }
  i = 0;
  int pfound = 0;
  while (cmd[i] != NULL) {
    if (strcmp(cmd[i], "|") == 0) {
      pfound = 1;
      j = 0;
      i++;
      continue;
    }
    if (pfound) {
      right[j] = cmd[i];
    } else {
      left[j] = cmd[i];
    }
    i++;
    j++;
  }
  if (pipe(pipeFD) < 0) {
    printf("\nError in creating pipe");
    return;
  }
  p1 = fork();
  if (p1 < 0) {
    printf("\nError during fork");
    exit(EXIT_FAILURE);
  }
  if (p1 == 0) {
    close(pipeFD[0]);
    dup2(pipeFD[1], STDOUT_FILENO);
    close(pipeFD[1]);
    if (execvp(*left, left) < 0) {
      printf("\nCould not execute command 1..");
      exit(0);
    }
  } else {
    // Parent executing
    p2 = fork();
    if (p2 < 0) {
      printf("\nError during fork");
      exit(EXIT_FAILURE);
    }
    // Child 2 executing..
    // It only needs to read at the read end
    if (p2 == 0) {
      close(pipeFD[1]);
      dup2(pipeFD[0], STDIN_FILENO);
      close(pipeFD[0]);
      if (execvp(*right, right) < 0) {
        printf("\nCould not execute command 2..");
        exit(0);
      }
    } else {
      // parent executing, waiting for two children
      wait(NULL);
      wait(NULL);
    }
  }
}

void runCmd(char **cmd, bool should_wait) {
  pid_t pid = fork();
  if (pid < 0) {
    perror("Error during fork");
    exit(EXIT_FAILURE);
  }
  if (pid == 0) {
    //sleep(2);
    if (execvp(*cmd, cmd) < 0)
      printf("Could not execute command");
    exit(EXIT_SUCCESS);
  }
  if (should_wait) {
    int status;
    waitpid(pid, &status, 0);
  }
}

int main() {
  char *args[MAX_LINE / 2 + 1];/* command line arguments */
  char *cmdLine = (char *) malloc(MAX_LINE * sizeof(char));
  for (int i = 0; i < MAX_LINE / 2 + 1; ++i)
    args[i] = NULL;
  char **currCmd = (char **) malloc(MAX_LINE * sizeof(char *));
  while (1) {
    printf("osh> ");
    fflush(stdout);
    int len = readline(&cmdLine);
    if (len <= 0)
      break;
    if (strcmp(cmdLine, "") == 0)
      continue;
    if (strcmp(cmdLine, "exit") == 0)
      break;
    int num_of_tokens = tokenize(cmdLine, args);
    int i = 0;
    int j = 0;
    for (int i = 0; i <= num_of_tokens; ++i)
      currCmd[i] = NULL;
    while (args[i] != NULL) {
      currCmd[j] = args[i];
      if (strcmp(args[i], "|") == 0) {
        pipedCmd(args);
        continue;
      }
      if (strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0) {
        redirect(args, true);
        continue;
      }
      if (strcmp(args[i], "&") == 0) {
        currCmd[j] = NULL;
        runCmd(currCmd, false);
        j = 0;
        i++;
        currCmd = (char **) malloc(MAX_LINE * sizeof(char *));
        for (int i = 0; i <= num_of_tokens; ++i)
          currCmd[i] = NULL;
        continue;
      }
      if (strcmp(args[i], ";") == 0) {
        currCmd[j] = NULL;
        runCmd(currCmd, true);
        j = 0;
        i++;
        currCmd = (char **) malloc(MAX_LINE * sizeof(char *));
        for (int i = 0; i <= num_of_tokens; ++i)
          currCmd[i] = NULL;
        continue;
      }
      if (args[i + 1] == NULL) {
        runCmd(currCmd, true);
      }
      i++;
      j++;
    }
    for (int i = 0; i <= num_of_tokens; ++i)
      args[i] = NULL;
  }
  printf("Exiting shell\n");
  return 0;
}
