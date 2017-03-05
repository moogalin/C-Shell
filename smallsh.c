#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Global */
pid_t parent_PID;
char * message;
char * args[512];
char first;
char last;
int numArgs = 0;
int statusVal = 0;
char statusMsg[30];
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

void status(pid_t fgPid) {
	printf(statusMsg);
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
 
//	printf("temp: %s\n", temp);
//	fflush(stdout);

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


void displayPIDS() {
	int i;

	for (i=0; i < 400; i++) {
		if (background_PIDS[i] != -1) {
			printf("BG_PID[%d] = %i\n", i, background_PIDS[i]);
			fflush(stdout);
		}
	}



}

void checkPIDS() {
	int i, exitStatus, termSignal = -1;
	int childExitMethod = -5;

	for (i=0; i < 400; i++) {
		if (background_PIDS[i] != -1) {
	
			/* check if PID is completed */
			if (waitpid(background_PIDS[i],&childExitMethod, WNOHANG)) {
				printf("background pid %i is done: ", background_PIDS[i]);
				fflush(stdout);			

				if (WIFEXITED(childExitMethod)) {
					exitStatus = WEXITSTATUS(childExitMethod);
					printf("exit value %d\n", exitStatus);
					fflush(stdout);
				}			
				else {
					termSignal = WTERMSIG(childExitMethod);	
					printf("terminated by signal %d\n", termSignal);
					fflush(stdout);				
				}

				background_PIDS[i] = -1;

			}  	

		}
	}




}

void redirectAndExecute(int foreground) {
	int i;
	char inFile[20], outFile[20];
	int inFD = -5;
	int outFD = -5;
	int devNull = -5;
	int result;
	char * params[10];
	int numParams = 0;	
	
	printf("redirectAndExecute\n");
	fflush(stdout);

	if (!foreground) {
		printf("bg process remove & arg\n");
		fflush(stdout);

		/* Prevent output and input from being read from terminal */
		devNull = open("/dev/null", O_RDWR);

		if (devNull == -1) {
			perror("devNull open()");
		}
		
		numArgs--;
	}

	args[numArgs] = NULL;


	/* Perform redirection */
	
	for (i=0; i < numArgs; i++) {
		if ( strcmp(args[i], ">") == 0 ) {
			printf("args[%d] is >\n", i);
			fflush(stdout);
	
			strcpy(outFile, args[i + 1]);

			printf("output file is: %s\n", outFile);
			fflush(stdout);

			outFD = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);

			if (outFD == -1) {
				printf("cannot open %s for output\n", outFile);
				fflush(stdout);
				statusVal = 1;	
				exit(1);	
			}	
			
			args[i] = NULL;
//			args[i+1] = " ";		
		}
		else if ( strcmp(args[i],"<") == 0 ) {
			printf("args[%d] is <\n", i);
			fflush(stdout);

			strcpy(inFile, args[i + 1]);

			printf("input file is: %s\n", inFile);
			fflush(stdout); 

			inFD = open(inFile, O_RDONLY);
			
			if (inFD == -1) {
				printf("cannot open %s for input\n", inFile);
				fflush(stdout);
				statusVal = 1;
				exit(1);
			}
			args[i] = NULL;
//			args[i+1] = "";

		}

		else {

		//	printf("else!\n");
		//	printf("numParams = %d and i = %d\n", numParams, i);
		//	params[numParams] =  args[i];
		//	printf("params[%d] = %s\n", numParams, params[numParams]);
		//	fflush(stdout);
		//	numParams++;		
	
		}

	}

	displayArgs();

	if (outFD != -5) {
		result = dup2(outFD, 1);
		if (result == -1) { perror("out dup2()"); exit(1); }
	}

	else {
		if (!foreground) {
			result = dup2(devNull, 1);
			if (result == -1) { perror("devNull dup2 out()"); exit(1); }			
		}
	}

	if (inFD != -5) {
		result = dup2(inFD, 0);
		if (result == -1) { perror("in dup2()"); exit(1); }	
	}

	else {
		if (!foreground) {
			result = dup2(devNull, 0);
			if (result == -1) { perror("devNull dup2 in()"); exit(1); }
		}
	
	}

/*	for (i = 1; i < numArgs; i++) {
			
		if (strcmp(args[i], "<")) {
			args[i] = NULL;
		}
		else {
	

	}
*/

//	printf("exec time\n");
//	fflush(stdout);

	if ( execvp(args[0],args) < 0 ) {
		perror(args[0]);
		exit(1);
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
	pid_t fgPid = -5;	
	int foreground = 1;
	int childExitMethod = -5;
	pid_t child_PID;
	int result;

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
	displayPIDS();

	/* Check BG pids array for ended processes */
	checkPIDS();

	/* Display list of background pids for debugging */

	foreground = 1;
	i = 0;

	/* shell prompt */
	printf(":");
	fflush(stdout);
	
	/* retrieve message */
	fflush(stdin);
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
//		printf("last char: %c\n", last);
		foreground = 0;
	}

//	printf("message: %s\n", message);
//	fflush(stdout);
	

	/* If cd command received, change working directory */
	if (strstr(message, "cd") == message) {
		callDirectory();
		continue;
	}

	/* If status command received, get status of last process */
	if (strstr(message, "status") == message) {
		status(fgPid);
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
//			printf("My parent: %i Me: %i\n", getppid(), getpid());
			fflush(stdout);

			redirectAndExecute(foreground);

			sleep(20);
			
			printf("done sleeping\n");
			fflush(stdout);
			exit(0);
	}

	
	if (foreground) {
		int exitStatus, termSignal;
//		printf("Foreground process!\n");
//		fflush(stdout);
	
		memset(statusMsg, '\0', sizeof(statusMsg));
		printf("PARENT: PID: %d, waiting...\n", spawnPid);
		fflush(stdout);

		result = waitpid(spawnPid, &childExitMethod, 0);

		if (result == -1) {
			perror("waitpid() failure:");
		}

		if (WIFEXITED(childExitMethod)) {
			exitStatus = WEXITSTATUS(childExitMethod);
			snprintf(statusMsg, sizeof(statusMsg), "exit value %d\n", exitStatus);
		//	fflush(stdout);
		}			

		else {
			termSignal = WTERMSIG(childExitMethod);	
			snprintf(statusMsg, sizeof(statusMsg), "terminated by signal %d\n", termSignal);
		//	fflush(stdout);				
		}
	


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
