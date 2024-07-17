#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/*
  Function Declarations for builtin shell commands:
 */
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  	"cd",
  	"help",
  	"exit"
};

int (*builtin_func[]) (char **) = {
  	&lsh_cd,
  	&lsh_help,
  	&lsh_exit
};

int lsh_num_builtins() {
  	return sizeof(builtin_str) / sizeof(char *);
}


/*
   Builtin command: change directory.
   args[0] is "cd".  args[1] is the directory.
 */
int lsh_cd(char **args){

  	if (args[1] == NULL) {
    		fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  	} else {
    		if (chdir(args[1]) != 0) {
      			perror("lsh");
    		}
  	}
  	return 1;
}

/*
   Builtin command: print help.
 */
int lsh_help(char **args){

  	int i;
  	printf("Stephen Brennan's LSH\n");
  	printf("Type program names and arguments, and hit enter.\n");
  	printf("The following are built in:\n");

  	for (i = 0; i < lsh_num_builtins(); i++) {
    		printf("  %s\n", builtin_str[i]);
  	}

  	printf("Use the man command for information on other programs.\n");
  	return 1;
}

/*
   Builtin command: exit.
   Always returns 0, to terminate execution.
 */
int lsh_exit(char **args){

  	return 0;
}

/*
    Launch a program and wait for it to terminate.
 */
int lsh_launch(char **args){

  	pid_t pid;
  	int status;

  	pid = fork();
  	if (pid == 0) {
    	// Child process
    		if (execvp(args[0], args) == -1) {
      		perror("lsh");
    		}
    		exit(EXIT_FAILURE);
  	} else if (pid < 0) {
    		// Error forking
    		perror("lsh");
  	} else {
    	// Parent process
    	do {
     		waitpid(pid, &status, WUNTRACED);
    	} while (!WIFEXITED(status) && !WIFSIGNALED(status));
  	}

  	return 1;
}

/*
	Execute shell built-in or launch program.
*/
int lsh_execute(char **args){

  	int i;

  	if (args[0] == NULL) {
    		return 1;
  	}

  	for (i = 0; i < lsh_num_builtins(); i++) {
    		if (strcmp(args[0], builtin_str[i]) == 0) {
      		return (*builtin_func[i])(args);
    		}
  	}

  	return lsh_launch(args);
}

/*
    Read a line of input from stdin. 
 */
char *lsh_read_line(void){

#ifdef LSH_USE_STD_GETLINE
  	char *line = NULL;
  	ssize_t bufsize = 0; // have getline allocate a buffer for us
  	if (getline(&line, &bufsize, stdin) == -1) {
    		if (feof(stdin)) {
      			exit(EXIT_SUCCESS);  // We received an EOF
    		} else  {
      			perror("lsh: getline\n");
      			exit(EXIT_FAILURE);
    		}
  	}
  	return line;
#else
	const int LSH_RL_BUFSIZE = 1024;

  	int bufsize = LSH_RL_BUFSIZE;
  	int position = 0;
  	char *buffer = malloc(sizeof(char) * bufsize);
  	int character;

  	if (!buffer) {
    		fprintf(stderr, "lsh: allocation error\n");
    		exit(EXIT_FAILURE);
  	}

  	while (1) {
    		character = getchar();

    		if (character == EOF) {
      			exit(EXIT_SUCCESS);
    		} else if (character == '\n') {
      			buffer[position] = '\0';
      			return buffer;
    		} else {
     			buffer[position] = character;
    		}
    			position++;

   	 	// If we have exceeded the buffer, reallocate.
    		if (position >= bufsize) {
      			bufsize += LSH_RL_BUFSIZE;
      			buffer = realloc(buffer, bufsize);
      			if (!buffer) {
        			fprintf(stderr, "lsh: allocation error\n");
        			exit(EXIT_FAILURE);
     			}
    		}
 	}
#endif
}


/*
   Split a line into tokens (very naively).
 */
char **lsh_split_line(char *line){

	const int LSH_TOK_BUFSIZE = 64;
	const char* LSH_TOK_DELIM = " \t\r\n\a";


  	int bufsize = LSH_TOK_BUFSIZE, position = 0;
  	char **tokens = malloc(bufsize * sizeof(char*));
  	char *token, **tokens_backup;

  	if (!tokens) {
    		fprintf(stderr, "lsh: allocation error\n");
    		exit(EXIT_FAILURE);
  	}

  	token = strtok(line, LSH_TOK_DELIM);
  	while (token != NULL) {
    		tokens[position] = token;
    		position++;

    		if (position >= bufsize) {
      			bufsize += LSH_TOK_BUFSIZE;
      			tokens_backup = tokens;
      			tokens = realloc(tokens, bufsize * sizeof(char*));
      			if (!tokens) {
				free(tokens_backup);
        			fprintf(stderr, "lsh: allocation error\n");
        			exit(EXIT_FAILURE);
      			}
    		}

    		token = strtok(NULL, LSH_TOK_DELIM);
  	}
  	tokens[position] = NULL;
  	return tokens;
}

/*
    Loop getting input and executing it.
 */
void lsh_loop(void){

  	char *line;
  	char **args;
  	bool status = 1;

	while (status){
		printf("> ");
		line = lsh_read_line();
		args = lsh_split_line(line);
		status = lsh_execute(args);
    		free(line);
    		free(args);
	}
}


int main(int argc, char **argv){
	// Load config files, if any.

	// Run command loop.
	lsh_loop();

	// Perform any shutdown/cleanup.

	return EXIT_SUCCESS;
}

