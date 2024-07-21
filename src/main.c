#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>



/*
    Launch a program and wait for it to terminate.
 */
bool lsh_launch(char **args){

  	pid_t pid;
  	int status;

  	pid = fork();
  	if (pid == 0) {
    	// Child process
    		if (execvp(args[0], args) == -1) {
      			perror("lsh");
    		}
    		return false;
  	} else if (pid < 0) {
    		// Error forking
    		perror("lsh");
		return false;
  	} else {
    	// Parent process     	
		waitpid(pid, &status, WUNTRACED);	
    		while (!WIFEXITED(status) && !WIFSIGNALED(status)){
			waitpid(pid, &status, WUNTRACED);
		}
		return true;
  	}

  	
}



/*
    Read a line of input from stdin. 
 */
char *lsh_read_line(void){

	const int LSH_RL_BUFSIZE = 1024;

  	int bufsize = LSH_RL_BUFSIZE;
  	int position = 0;
  	char *buffer = malloc(sizeof(char) * bufsize);
  	int character;

  	if (!buffer) {
    		fprintf(stderr, "lsh: allocation error\n");
    		exit(EXIT_FAILURE);
  	}

  	while (true) {
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

    		if (position >= bufsize) {
      			bufsize += LSH_RL_BUFSIZE;
      			buffer = realloc(buffer, bufsize);
      			if (!buffer) {
        			fprintf(stderr, "lsh: allocation error\n");
        			exit(EXIT_FAILURE);
     			}
    		}
 	}
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
bool lsh_loop(void){

  	char *line;
  	char **args;
	bool status = true;
	bool commandExec;

	while (status){
		printf("> ");
		line = lsh_read_line();
		args = lsh_split_line(line);

		if (args[0] == NULL) {
			perror("No arguments given");
			continue;
  		}

		if (args[0] == "exit") {
			perror("Exiting from the shell");			
			status = false;
  		}

		commandExec = lsh_launch(args);
		if (!commandExec){
			perror("There was an error in the execution of the program");
		} 

	
	}
	free(line);
    	free(args);
	return status;
}


int main(int argc, char **argv){
	// Parses shell arguments

	// Load config files, if any. Command History/Aliases/Enviroment Variables


	if (!(lsh_loop())) { // Parsing (Pipes / Quoting / Special Characters/ Wildcards / Shell Expansions)
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}

