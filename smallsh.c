#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global */
pid_t parent_PID;
char message[2048];


int isValidInput() {
	if (message[0] == '#' || message[0] == '\n' || message[0] == '\t' || message[0]  == ' ' ) {
//		printf("you entered a blank line, space, tab, or # sign\n");
//		fflush(stdout);
		return 0;
	}	
	else return 1;

}

void cleanup() {
//	printf("now exiting\n");
	fflush(stdout);
}

void callDirectory() {
	printf("first two characters are cd\n");
	fflush(stdout);	

	if (strcmp(message, "cd\n") == 0) {
		printf("Only characters are cd\n");
		fflush(stdout);
	}
	else {
		printf("There are args\n");
		fflush(stdout);
	}	
}

void status() {
	printf("first word is status\n");
	fflush(stdout);
	
}

int main() {

	parent_PID = getpid();


	/* Display shell until exit command received */
	while(1) {

	/* clear the character input array */
	memset(message, '\0', sizeof(message));

	/* shell prompt */
	printf(":");
	fflush(stdout);
	
	/* retrieve message */
	fgets(message, 2048, stdin);
	fflush(stdout);

	/* Check for tab, newline, or # symbol to begin user input */
	if (!isValidInput()) {
		continue;
	}


	/* If exit message received, terminate processes and exit shell */
	if (strcmp(message, "exit\n") == 0) {
		cleanup();
		return 0;	
	}

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
