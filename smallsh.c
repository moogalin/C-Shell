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
pid_t parent_PID; 		//Shell PID
char * message;			//String representing user input
char * args[512];		//Array of arguments from user input
char first;			//First character of first argument
char last;			//First character of last argument
int numArgs = 0;		//Assists in argument indexing		
char statusMsg[30];		//Save status message for FG process
pid_t background_PIDS[400];	//Save PIDs of background processes
int fgMode = 0;			//Bool representing if fgMode is active 

/* Catch and handle Ctrl-Z SIGTSTP signals */
void catchSIGTSTP(int signo) {
	
	if (!fgMode) {
		char * message = "Entering foreground-only mode (& is now ignored)\n";
		write(STDOUT_FILENO, message, 49);
		fgMode = 1;
	}
	else {
		char * message = "Exiting foreground-only mode\n";
		write(STDOUT_FILENO, message, 29);
	
		/* Reset fg only flag for next Ctrl-Z signal */
		fgMode = 0;
	}
	
}

/* Validate first character of user input (ie. ignore comments denoted by '#') */
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

/* This function kills all background processes to prepare for shell exit */
void cleanup() {

	int i, childExitMethod;

	for (i=0; i < 400; i++) {

		/* Ignore non-PID elements in array */
		if (background_PIDS[i] != -1) {
			kill(background_PIDS[i], SIGKILL );
			waitpid(background_PIDS[i], &childExitMethod, 0);
		}

	} 

}

/* This function changes the current working directory of the shell */
void callDirectory() {

	char cwd[1024];
	
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		perror("getcwd() error");
		exit;
	}
	

	/* No arguments are passed, change current working directory to 'Home' env variable */
	if (strcmp(message, "cd") == 0) {

		chdir(getenv("HOME"));
	
		if (getcwd(cwd, sizeof(cwd)) == NULL) {
 			perror("getcwd() error");
			exit;
		}	
	
	}
	
	/* 1+ arguemnts passed. First argument is the new directory location */
	else {

		chdir(args[1]);
	}	
}

/* This function prints the status message of the last foreground process */
void status() {
	
	printf(statusMsg);
	fflush(stdout);

}

/* This function converts a terminal message into an array of string arguments for shell commands */
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

	/* Retrieve words from the temp string */
	token = strtok(temp, " ");

	while (	token != NULL) {
		
		/* Save word from temp string into argument array */
		strcpy(args[i], token);
		i++;

		token = strtok(NULL, " ");
		
	}
	
	/* Last argument is null string */
	args[i] = NULL;
	
	/* Save number of arguments */
	numArgs = i;

	/* Get first character of first argument */
	first = *args[0];
	
	/* Get first character of last argument */
	last = *args[numArgs-1];

}

/* This function will replace '$$' with the PID of the executing shell */
void appendPID() {

	int j,k,l;

	/* Repeat process for each command line argument */
	for (j=0; j < numArgs; j++) {
	
		/* Save current string length of argument before it changes */
		l = strlen(args[j])-1;
		
		/* Look through every character in argument */
		for (k=0; k<l; k++) {
			
			/* Check if character and subsequent character are the same */
			if (*(args[j]+k) == *(args[j]+k+1)) {
				/* If the same, check if character is '$,' this means we have "$$" */
				if (*(args[j]+k) == '$') {
					/* Append shell PID over first '$' */
					sprintf(args[j]+k, "%d",parent_PID);

				}			
			}
		}
	}
}

/* For debugging purposes, this function displays all command line arguments */
void displayArgs() {
	int i=0;

	for (i=0; i < numArgs; i++) {
		printf("arg[%d] = %s\n", i, args[i]);
		fflush(stdout);
	}	
}

/* This function adds a new background PID to the array of currently running PIDS */
void addToArray(pid_t pid) {
	int i;

	for (i =0; i < 400; i++) {
		
		/* Only add PID to empty array element */
		if (background_PIDS[i] == -1) {
			background_PIDS[i] = pid;
			return;		
		}
	}
}

/* For debugging purposes, this function displays all currently active background PIDs */
void displayPIDS() {
	int i;

	for (i=0; i < 400; i++) {
		if (background_PIDS[i] != -1) {
			printf("BG_PID[%d] = %i\n", i, background_PIDS[i]);
			fflush(stdout);
		}
	}



}

/* Iterate through background PIDs array to check if the background process is completed */
void checkPIDS() {
	int i, exitStatus, termSignal = -1;
	int childExitMethod = -5;

	for (i=0; i < 400; i++) {
		if (background_PIDS[i] != -1) {
	
			/* check if PID is completed */
			if (waitpid(background_PIDS[i],&childExitMethod, WNOHANG)) {
				printf("background pid %i is done: ", background_PIDS[i]);
				fflush(stdout);			

				/* PID exited, display exit value */
				if (WIFEXITED(childExitMethod)) {
					exitStatus = WEXITSTATUS(childExitMethod);
					printf("exit value %d\n", exitStatus);
					fflush(stdout);
				}			
				/* PID terminated via signal, display signal */
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

/* Redirect input and output of child process (if appropriate) and then execute new program */
void redirectAndExecute(int foreground) {
	int i;
	char inFile[20], outFile[20];
	int inFD = -5;
	int outFD = -5;
	int devNull = -5;
	int result;
	char * params[10];
	int numParams = 0;	
	
	/* For background processes, keep I/O from terminal */
	if (!foreground) {
		

		/* Prevent output and input from being read from terminal */
		devNull = open("/dev/null", O_RDWR);

		if (devNull == -1) {
			perror("devNull open()");
		}
		
		/* Clear '&' from arguments list */
		numArgs--;
	}

	/* Exec expects last argument to be NULL string */
	args[numArgs] = NULL;


	/* Open up input/output files for reading/writing, if necessary */
	for (i=0; i < numArgs; i++) {
		
		/* Redirect output to file path specified by argument following '>' */
		if ( strcmp(args[i], ">") == 0 ) {
	
			strcpy(outFile, args[i + 1]);

			outFD = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);

			if (outFD == -1) {
				printf("cannot open %s for output\n", outFile);
				fflush(stdout);
				exit(1);	
			}	
			
			/* Prevent '>' character and subsequent arguments from being used in exec. */ 
			args[i] = NULL;
		}
		/* Redirect input to file path specified by argument following '<' */
		else if ( strcmp(args[i],"<") == 0 ) {

			strcpy(inFile, args[i + 1]);

			inFD = open(inFile, O_RDONLY);
			
			if (inFD == -1) {
				printf("cannot open %s for input\n", inFile);
				fflush(stdout);
				exit(1);
			}
			
			/* Prevent '<' character and subsequent arguments from being used in exec. */ 
			args[i] = NULL;

		}


	}
	
	/* Perform redirection */

	/* Output file specified, redirect output to write file */
	if (outFD != -5) {
		result = dup2(outFD, 1);
		if (result == -1) { perror("out dup2()"); exit(1); }
	}

	/* Output file not specified, if background process, redirect output to dev/null */
	else {
		if (!foreground) {
			result = dup2(devNull, 1);
			if (result == -1) { perror("devNull dup2 out()"); exit(1); }			
		}
	}
	
	/* Input file specified, redirect input from read file */
	if (inFD != -5) {
		result = dup2(inFD, 0);
		if (result == -1) { perror("in dup2()"); exit(1); }	
	}

	/* Input file not specified, if background process, redirect input from dev/null */
	else {
		if (!foreground) {
			result = dup2(devNull, 0);
			if (result == -1) { perror("devNull dup2 in()"); exit(1); }
		}
	
	}

	/* Pass array of arguments to execvp and execute new program. */
	if ( execvp(args[0],args) < 0 ) {
		perror(args[0]);
		exit(1);
	}
}

/* Main entry point for function */
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
	struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0};
	SIGINT_action.sa_handler = SIG_IGN;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = 0;

	SIGTSTP_action.sa_handler = catchSIGTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = SA_RESTART;

	/* Signal actions in shell */
	sigaction(SIGINT, &SIGINT_action, NULL);
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

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

	/* Determine if process will run in background or foreground and check if fgMode is on */
	if ((last == '&') && (strcmp(args[0], "echo") != 0) ) {
		/* Process will run in background */
		if (!fgMode) {
			foreground = 0;
		}
		
		/* fgMode is on. All background processes will run in foreground */
		else {		
			/* Hide '&' and report as foreground process */
			numArgs--;
			foreground = 1;
		}
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

			/* Child process will perform default action (stop) on Ctrl-C SIGINT */
			SIGINT_action.sa_handler = SIG_DFL;
			sigaction(SIGINT, &SIGINT_action, NULL);
		
			/* Child process will ignore Ctrl-Z SIGTSTP */
			SIGTSTP_action.sa_handler = SIG_IGN;
			sigaction(SIGTSTP, &SIGTSTP_action, NULL);

			/* Perform I/O redirection and execute command line arguments */
			redirectAndExecute(foreground);

			/* Clean exit on success */
			exit(0);
	}

	/* Foreground process, parent shell processing */
	if (foreground) {

		int exitStatus, termSignal;
	
		memset(statusMsg, '\0', sizeof(statusMsg));

		result = waitpid(spawnPid, &childExitMethod, 0);

		if (result == -1) {
			perror("waitpid() failure:");
		}

		if (WIFEXITED(childExitMethod)) {
			exitStatus = WEXITSTATUS(childExitMethod);
			/* Save status message for foreground process exit */ 
			snprintf(statusMsg, sizeof(statusMsg), "exit value %d\n", exitStatus);
		}			

		else {
			termSignal = WTERMSIG(childExitMethod);	
			/* Save status message for foreground process terminated via signal */
			snprintf(statusMsg, sizeof(statusMsg), "terminated by signal %d\n", termSignal);
			if (termSignal == 2) {
				/* If signal terminated via Signal 2 (Ctrl-C) print status message to console */
				printf(statusMsg);
			}
		}
	


	}
	
	/* Background process, parent shell processing */
	else {
		printf("background pid is %d\n", spawnPid);
		fflush(stdout);		

		addToArray(spawnPid);
		continue;
	}
	}
}
