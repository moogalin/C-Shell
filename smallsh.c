#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


/* Global */
pid_t parent_PID;
char * message;
char * args[512];
char first;
char last;
int numArgs = 0;
char statusMsg[20];


int isValidInput() {
	if ( first == '#') {
//		printf("you entered a # sign\n");
//		fflush(stdout);
		return 0;
	}

	else if (first == '\n') {
//		printf("you entered a blank line\n");
//		fflush(stdout);
		return 0;
	}
	else if (first == '\t' && numArgs == 1) {
//		printf("you entered a tab\n");
//		fflush(stdout);
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
	//	chdir(
	}	
}

void status() {
	printf("first word is status\n");
	fflush(stdout);
	
}

void getArgs(char * temp) {
	int i;
	char * token;

	/* Get number of arguments and assign to arg array */
	for (i = 0; i < 512; i++) {
		args[i] = malloc(30);

	}
	i = 0;

	token = strtok(temp, " ");

	while (	token != NULL) {
		args[i] = token;
		i++;
//		printf("args[%d]=%s", i-1, token);
//		fflush(stdout);

	token = strtok(NULL, " ");
		
	}

	args[i] = NULL;
	numArgs = i;

	/* Get first character of first argument */
	first = *args[0];
	
	/* Get first character of last argument */
	last = *args[numArgs-1];



}

void appendPID() {

	int j,k,l;
//	printf("arg[0] length is: %d\n", strlen(args[1]));
	for (j=0; j < numArgs; j++) {
	
		l = strlen(args[j])-1;
		for (k=0; k<l; k++) {
			if (*(args[j]+k) == *(args[j]+k+1)) {
				if (*(args[j]+k) == '$') {
//					printf("arg: %s\n", args[j]);
//					fflush(stdout);
				
//					printf("argj + k = %s", args[j]+k);
					fflush(stdout);

					sprintf(args[j]+k, "%d",parent_PID);

//					printf("arg: %s\n",args[j]);
//					fflush(stdout);

				}
				
			}
//				printf("%d %d = %c%c\n ", k,k+1,*(args[j]+k),*(args[j]+k+1));
//				fflush(stdout);
		}


	}

}

void displayArgs() {
	int i=0;


	for (i=0; i < numArgs; i++) {

		printf("arg[%d] = %s\n", i, args[i]);
		fflush(stdout);
	}
	

}

int main( int argc, char * argv[]) {

	parent_PID = getpid();
	ssize_t buffer = 0;	
	char * token;
	char temp[2048]; 
	int i = 0, j, k, l;
	int charwrit;

	/* Display shell until exit command received */
	while(1) {
	i = 0;

	/* shell prompt */
	printf(":");
	fflush(stdout);
	
	/* retrieve message */
	getline(&message, &buffer, stdin);

	/* If exit message received, terminate processes and exit shell */
	if (strcmp(message, "exit\n") == 0) {
		cleanup();
		return 0;	
	}

	/* Copy message to temp variable to get arguments */
	strcpy(temp, message);

	printf("Message: %s\n", message);
	fflush(stdout);
 
	printf("temp: %s\n", temp);
	fflush(stdout);


	/* Get number of arguments and assign to arg array */
	getArgs(temp);

	/* Check for tab, newline, or # symbol to begin user input */
	if (!isValidInput()) {
		continue;
	}

	/* Look through arguments and replace $$ with shell pid */
	appendPID();

	/* display args for debugging */
	displayArgs();

//	printf("first char: %c\n", first);

	if (last == '&') {
		printf("last char: %c\n", last);
	}

	printf("message: %s", message);
	fflush(stdout);
	

	/* If cd command received, change working directory */
	if (strstr(message, "cd") == message) {
		callDirectory();
		continue;
	}

	/* If status command received, get status of last process */
	if (strstr(message, "status") == message) {
		status();
		continue;
	}


	}

}
