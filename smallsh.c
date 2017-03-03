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
pid_t background_PIDS[400];




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

	if (strcmp(message, "cd") == 0) {
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
//		printf("arg1: %s\n", args[1]);
//		fflush(stdout);

		chdir(args[1]);
	}	
}

void status() {
	printf("first word is status\n");
	fflush(stdout);
	
}

void getArgs() {
	int i;
	char * token;
	char temp[2048];

	/* Remove new line character from message */
	message[strlen(message)-1] = '\0';

	/* Copy message to temp variable to get arguments */
	strcpy(temp, message);

	printf("Message: %s\n", message);
	fflush(stdout);
 
	printf("temp: %s\n", temp);
	fflush(stdout);

	/* Get number of arguments and assign to arg array */
	for (i = 0; i < 512; i++) {
		args[i] = malloc(30);

	}
	i = 0;

	token = strtok(temp, " ");

	while (	token != NULL) {
		//args[i] = token;
		strcpy(args[i], token);
		i++;
	//	printf("token[%d]=%s, args=%s", i-1, token, args[i-1]);
	//	fflush(stdout);

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

void addToArray(pid_t pid) {
	int i;

	for (i =0; i < 400; i++) {

		if (background_PIDS[i] == -1) {
			background_PIDS[i] = pid;
			return;		
		}
	}





}

int main( int argc, char * argv[]) {

	parent_PID = getpid();
	ssize_t buffer = 0;	
	char * token;
	char temp[2048]; 
	int i = 0, j, k, l;
	int charwrit;
	pid_t spawnPid = -5;	
	int foreground = 1;
	int childExitMethod = -5;
	pid_t child_PID;

	/* Print shell PID */
	printf("Shell PID: %i\n", parent_PID);
	fflush(stdout);

	/* Initialize array of PIDs to zero */
	for (i =0; i < 400; i++) {
	
		background_PIDS[i] = -1;
	}


	/* Display shell until exit command received */
	while(1) {

	/* Display list of background pids for debugging */
	for (i=0; i < 400; i++) {
		if (background_PIDS[i] != -1) {
			printf("BG_PID[%d] = %i\n", i, background_PIDS[i]);
			fflush(stdout);
			
			printf("Checking if exists \n");
			fflush(stdout);

			
		}
	}

	foreground = 1;
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

	/* Get number of arguments and assign to arg array */
	getArgs();

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
		foreground = 0;
	}

	printf("message: %s\n", message);
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

	spawnPid = fork();

	if (spawnPid == -1) {	
			perror("Hull breach!");
			exit(1);
	}
	else if (spawnPid == 0) {
			printf("I am the child process in action\n");
			fflush(stdout);
			printf("My parent: %i Me: %i\n", getppid(), getpid());
			fflush(stdout);

			sleep(10);
			exit(0);
	}

	
	if (foreground) {
		printf("Foreground process!\n");
		fflush(stdout);
	
		printf("PARENT: PID: %d, waiting...\n", spawnPid);
		fflush(stdout);

		waitpid(spawnPid, &childExitMethod, 0);

		printf("PARENT: Child process terminated, exiting!\n");
		fflush(stdout);	
	}
	else {
		printf("background pid is %d\n", spawnPid);
		fflush(stdout);		

		addToArray(spawnPid);
		continue;
	}

	}

}
