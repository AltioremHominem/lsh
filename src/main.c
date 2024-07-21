#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

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


bool lsh_exit(char **args){

  	return 0;
}

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
    		exit(EXIT_FAILURE);
  	} else if (pid < 0) {
    	// Error forking
    	perror("lsh");
	} else {
    	// Parent process     	
		waitpid(pid, &status, WUNTRACED);	
    		while (!WIFEXITED(status) && !WIFSIGNALED(status)){
			waitpid(pid, &status, WUNTRACED);
		}
  	}
	return true;
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

	while (true){
		printf("> ");
		line = lsh_read_line();
		args = lsh_split_line(line);

		if (args[0] == NULL) {
			continue;
  		}

		for (int i = 0; i < lsh_num_builtins(); i++) {
    			if (strcmp(args[0], builtin_str[i]) == 0) {
      				return (*builtin_func[i])(args);
    			}
  		}

		if (!lsh_launch(args)){
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

