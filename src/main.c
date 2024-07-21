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
int lsh_launch(char **args){

  	pid_t pid;
  	int status;

  	pid = fork();
  	if (pid == 0) {
    	// Child process
    		if (execvp(args[0], args) == -1) {
      			perror("lsh1");
    		}
		return 0;
  	} else if (pid < 0) {
    		// Error forking
    		perror("lsh2");
  	} else {
    	// Parent process     	
		waitpid(pid, &status, WUNTRACED);	
    		while (!WIFEXITED(status) && !WIFSIGNALED(status)){
			waitpid(pid, &status, WUNTRACED);
		}
  	}
	return 1;
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
			fflush(stdin);
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
  	char *token;

  	if (!tokens) {
    		fprintf(stderr, "lsh: allocation error\n");
    		exit(EXIT_FAILURE);
  	}

  	token = strtok(line, LSH_TOK_DELIM);
  	while (token != NULL) {
		if (sizeof(tokens) <= sizeof(token) ){
			bufsize += LSH_TOK_BUFSIZE;
			tokens = realloc(tokens, bufsize * sizeof(char*));
      			if (!tokens) {
        			fprintf(stderr, "lsh: allocation error\n");
        			exit(EXIT_FAILURE);
      			}	
		}

    		tokens[position] = token;
    		position++;

    		if (position >= bufsize) {
      			bufsize += LSH_TOK_BUFSIZE;
      			tokens = realloc(tokens, bufsize * sizeof(char*));
      			if (!tokens) {
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
	bool commandExec = false;

	while (true){
		printf("> ");
		line = lsh_read_line();
		args = lsh_split_line(line);

		if (args[0] == NULL) {
			continue;
  		}

		if (strcmp(args[0],"exit") == 0)  {
			printf("Exiting from the shell");		
			free(line);
    			free(args);
			exit(EXIT_FAILURE);
  		}

		commandExec = lsh_launch(args);
		if (!commandExec){
			printf("There was an error in the execution of the program");
		} 
		free(line);
    		free(args);	
	}
	
}


int main(int argc, char **argv){
	// Parses shell arguments

	// Load config files, if any. Command History/Aliases/Enviroment Variables


	lsh_loop(); // Parsing (Pipes / Quoting / Special Characters/ Wildcards / Shell Expansions)
	
	
	return EXIT_SUCCESS;
}

