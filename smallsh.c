#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


/* Global */
pid_t parent_PID;
char * message;
char ** args;
char first;
char last;
int numArgs = 0;

int isValidInput() {
	if ( first == '#') {
		printf("you entered a # sign\n");
		fflush(stdout);
		return 0;
	}

	else if (first == '\n') {
		printf("you entered a blank line\n");
		fflush(stdout);
		return 0;
	}
	else if (first == '\t' && numArgs == 1) {
		printf("you entered a tab\n");
		fflush(stdout);
		return 0;
	} 

	else return 1;

}

void cleanup() {
	printf("now exiting\n");
	fflush(stdout);
}

void callDirectory() {

	char cwd[1024];
	
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		perror("getcwd() error");
		exit;
	}
	
	printf("CWD: %s\n", cwd);
	fflush(stdout);

//	printf("first two characters are cd\n");
//	fflush(stdout);	

	if (strcmp(message, "cd\n") == 0) {
//		printf("Only characters are cd\n");
//		fflush(stdout);

		printf("Home environment: %s\n", getenv("HOME"));
		fflush(stdout);

		chdir(getenv("HOME"));
	
		if (getcwd(cwd, sizeof(cwd)) == NULL) {
 			perror("getcwd() error");
			exit;
		}	
	
		printf("CWD: %s\n", cwd);
		fflush(stdout); 
	}
	else {


	
//		printf("There are args\n");
//		fflush(stdout);
	}	
}

void status() {
	printf("first word is status\n");
	fflush(stdout);
	
}

int main( int argc, char * argv[]) {

	parent_PID = getpid();
	ssize_t buffer = 0;	
	char * token;
	char * temp = malloc(2048); 
	int i = 0;

	/* Display shell until exit command received */
	while(1) {
	i = 0;

	/* clear the character input array */
//	memset(message, '\0', sizeof(message));

	/* shell prompt */
	printf(":");
	fflush(stdout);
	
	/* retrieve message */
//	fgets(message, 2048, stdin);
//	fflush(stdout);
	getline(&message, &buffer, stdin);


	printf("Message: %s\n", message); 

	/* If exit message received, terminate processes and exit shell */
	if (strcmp(message, "exit\n") == 0) {
		cleanup();
		return 0;	
	}

	/* Get number of arguments */
	args = malloc(512 * sizeof(char*));

	token = strtok(message, " ");

	while (	token != NULL) {
	
		args[i] = token;
		i++;
		printf("args[%d]=%s", i-1, token);
		fflush(stdout);

	token = strtok(NULL, " ");
		
	}

	args[i] = NULL;
	numArgs = i;

	/* Get first character of first argument */
	first = *args[0];
	
	/* Get first character of last argument */
	last = *args[i-1];

	printf("first char: %c\n", first);

	if (last == '&') {
		printf("last char: %c\n", last);
	}

//	printf("message: %s", message);
//	fflush(stdout);
	

	/* Check for tab, newline, or # symbol to begin user input */
	if (!isValidInput()) {
		continue;
	}
	else {
		printf("valid\n");
		fflush(stdout);
	}
}



	/* If cd command received, change working directory */
//	if (strstr(message, "cd") == message) {
//		callDirectory();
//		continue;
//	}

	/* If status command received, get status of last process */
//	if (strstr(message, "status") == message) {
//		status();
//		continue;
//	}

	free(message);
	free(args);

}
