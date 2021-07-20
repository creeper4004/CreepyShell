#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//Function declarations
int cshell_cd(char **args);
int cshell_help(char **args);
int cshell_exit(char **args);

// List of builtin commands, followed by their corresponding functions
char *builtin_str[] = { "cd", "help", "exit" };

int (*builtin_func[]) (char **) = { &cshell_cd, &cshell_help, &cshell_exit };

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

//Builtin functions implementation

int cshell_cd(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "CreepyShell: expected argument to work with cd\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("creepy_shell");
    }
  }
  return 1;
}

int cshell_help(char **args) {
  printf("CreepyShell\n");
  printf("Write program names and arguments, and hit enter.\n");
  printf("The followed are builtin commands: \n");
  for ( int i = 0; i < lsh_num_builtins(); i++ ) {
    printf(" %s\n", builtin_str[i]);
  }
  printf("Use the man commands for information on other commands");
  return 1;
}

int cshell_exit(char **args) {
  printf("CreepyShell say to you Bye :D");
  return 0;
}

int cshell_launch(char **args) {
  pid_t pid = fork();
  int status;
  if (pid == 0) {
    //child process
    if (execvp(args[0], args) == -1) {
      perror("creepy_shell");
    } 
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    //error forking
    perror("creepy_shell");
  } else {
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status)&& !WIFSIGNALED(status));
  }
  return 1;
}

int cshell_execute(char **args) {
  if (args[0] == NULL) {
    // an empty command was passed
    return 1;
  }
  for (int i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }
  return cshell_launch(args);
}

char *cshell_read_line(void) {
#ifdef CSHELL_USE_STD_GETLINE
  char *line = NULL;
  ssize_t bufsize = 0;
  if (getline(&line, &buf, stdin) == -1) {
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);
    } else {
      perror("creepy_shell: getline");
      exit(EXIT_FAILURE);
    }
  }
  return line;
#else
#define CSHELL_RL_BUFSIZE 1024
  int bufsize = CSHELL_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "CreepyShell: Allocation error\n");
    exit(EXIT_FAILURE);
  }
  while (1) {
    //read chr
    c = getchar();

    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;
    // if we have exceed the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += CSHELL_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "CreepyShell: Allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
#endif
}

#define CSHELL_TOK_BUFSIZE 64
#define CSHELL_TOK_DELIM " \t\r\n\a"

char **cshell_split_line(char *line) {
  int bufsize = CSHELL_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "CreepyShell: Allocation error\n");
    exit(EXIT_FAILURE);
  }
  token = strtok(line, CSHELL_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += CSHELL_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        free(tokens_backup);
        fprintf(stderr, "CreepyShell: Allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
    token = strtok(NULL, CSHELL_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens ;
}

//main loop to get input and execute them

void cshell_loop(void) {
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = cshell_read_line();
    args = cshell_split_line(line);
    status = cshell_execute(args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv) {
  //should load config files if any
  //no config files at the moment

  //stat the loop
  cshell_loop();

  //perform any shutdown/cleanup if needed
  // and we fninally close
  return EXIT_SUCCESS;
}
