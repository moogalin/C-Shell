#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

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


void catchSIGINT(int signo) {
	char * message = "Caught SIGINT, sleeping for 5 seconds\n";
	write(STDOUT_FILENO, message, 38);
	raise(SIGUSR2);
	sleep(5);
}

/*void catchSIGUSR2(int signo) {
	char * message = "Caught SIGUSR2, exiting!\n";
	write(STDOUT_FILENO, message, 25);
	exit(0);

}*/

int isValidInput() {
	if ( first == '#') {
		return 0;
	}

	else if (first == '\n') {
		return 0;
	}
	else if (first == '\t' && numArgs == 1) {
		return 0;
	} 

	else return 1;

}

void cleanup() {

	kill(0, SIGKILL);
}

void callDirectory() {

	char cwd[1024];
	
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		perror("getcwd() error");
		exit;
	}
	

	if (strcmp(message, "cd") == 0) {

		chdir(getenv("HOME"));
	
		if (getcwd(cwd, sizeof(cwd)) == NULL) {
 			perror("getcwd() error");
			exit;
		}	
	
	}
	else {

		chdir(args[1]);
	}	
}

void status() {
	
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

	/* Get number of arguments and assign to arg array */
	for (i = 0; i < 512; i++) {
		args[i] = malloc(30);

	}
	i = 0;

	token = strtok(temp, " ");

	while (	token != NULL) {
	
		strcpy(args[i], token);
		i++;

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

	for (j=0; j < numArgs; j++) {
	
		l = strlen(args[j])-1;
		for (k=0; k<l; k++) {
			if (*(args[j]+k) == *(args[j]+k+1)) {
				if (*(args[j]+k) == '$') {

					sprintf(args[j]+k, "%d",parent_PID);

				}
				
			}
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
	
	if (!foreground) {

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
	
			strcpy(outFile, args[i + 1]);

			outFD = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);

			if (outFD == -1) {
				printf("cannot open %s for output\n", outFile);
				fflush(stdout);
				exit(1);	
			}	
			
			args[i] = NULL;
		}
		else if ( strcmp(args[i],"<") == 0 ) {

			strcpy(inFile, args[i + 1]);

			inFD = open(inFile, O_RDONLY);
			
			if (inFD == -1) {
				printf("cannot open %s for input\n", inFile);
				fflush(stdout);
				exit(1);
			}
			args[i] = NULL;

		}


	}


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

	/*Initialize Signal Handlers */
	struct sigaction SIGINT_action = {0};
//	SIGINT_action.sa_handler = catchSIGINT;
	SIGINT_action.sa_handler = SIG_IGN;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = 0;

	/* Signal actions in shell */
	sigaction(SIGINT, &SIGINT_action, NULL);


//	sigfillset(&SIGUSR2_action.sa_mask);
//	SIGUSR2_action.sa_flags = 0;
//	sigaction(SIGUSR2, &SIGUSR2_action, NULL);
//	sigaction(SIGUSR2, &ignore_action, NULL);
//	sigaction(SIGTERM, &ignore_action, NULL);
//	sigaction(SIGHUP, &ignore_action, NULL);
//	sigaction(SIGQUIT, &ignore_action, NULL);

	

	/* Initialize array of PIDs to zero */
	for (i =0; i < 400; i++) {
	
		background_PIDS[i] = -1;
	}


	/* Display shell until exit command received */
	while(1) {

	/* Display list of background pids for debugging */
//	displayPIDS();

	/* Check BG pids array for ended processes */
	checkPIDS();


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


	if ((last == '&') && (strcmp(args[0], "echo") != 0) ) {
		foreground = 0;
	}


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

			SIGINT_action.sa_handler = SIG_DFL;
			sigaction(SIGINT, &SIGINT_action, NULL);

			redirectAndExecute(foreground);

			exit(0);
	}

	
	if (foreground) {
		int exitStatus, termSignal;
	
		memset(statusMsg, '\0', sizeof(statusMsg));

		result = waitpid(spawnPid, &childExitMethod, 0);

		if (result == -1) {
			perror("waitpid() failure:");
		}

		if (WIFEXITED(childExitMethod)) {
			exitStatus = WEXITSTATUS(childExitMethod);
			snprintf(statusMsg, sizeof(statusMsg), "exit value %d\n", exitStatus);
		}			

		else {
			termSignal = WTERMSIG(childExitMethod);	
			snprintf(statusMsg, sizeof(statusMsg), "terminated by signal %d\n", termSignal);
			if (termSignal == 2) {
				printf(statusMsg);
			}
		}
	


	}
	else {
		printf("background pid is %d\n", spawnPid);
		fflush(stdout);		

		addToArray(spawnPid);
		continue;
	}

	}

}
