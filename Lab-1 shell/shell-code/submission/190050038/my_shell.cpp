#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <set>


// [DONE] /* Might have to handle mem leak for child processes */
// [DONE] /* Handle signal for all processes */ DONE 

#define MAX_INPUT_SIZE 1024 
#define MAX_TOKEN_SIZE 64   
#define MAX_NUM_TOKENS 64   
#define MAX_NUM_COMMANDS 64 

std::set<int> backProcesses; //for storing the background process PIDs
bool carrySerialCommands = true;


void exitCall(char** tokens, char ** commands);
bool isExitCmd(char ** tokens);
void reapChildren();
char ** tokenize(char *line);
char ** breakIntoCommands(char* line);
void freeTokens(char ** tokens);
void freeCommands(char ** commands);
int execSingleCommandForeSerial(char **tokens, char ** commands);
int execSingleCommandBack(char **tokens, char ** commands);
int execMultiCommandForeParallel(char **tokens, char ** commands);
void reapChildrenSurely();

/* Function to handle the "exit" command. Frees heap. 
* SIGTERM to children processes with same pgid. (Process Group ID)
*/
void exitCall(char** tokens, char ** commands){
	// [DONE] clean the heap allocations
	// [DONE] kill the childrens!!! with same pgid
	// [DONE] exit on current process... SIGTERM CALLED

	freeCommands(commands);
	freeTokens(tokens);
	for(int i : backProcesses){
		kill(i, SIGTERM);
	}
	kill(getpid(), SIGTERM);
	// exit(1);	
	return;
}

/* SIGTERM signal handler. Waits and reaps all children and exit the process.
*/
void sigterm_handler(int sig){

	reapChildrenSurely(); //waits till termination of children and reaps all
	printf("Process MAIN [ %d ] terminated by SIGTERM\n", getpid());
	exit(0);
}


/* SIGINT handler for main 
*/
void sigint_handler(int sid){
	printf("\n");
	carrySerialCommands = false;
	reapChildren();
	return;
}

/* Bool, tells whether the command is exit or not 
*/
bool isExitCmd(char ** tokens){
	if(tokens[0]==NULL) return false;
	if(strcmp(tokens[0],"exit")==0 && tokens[1]==NULL) return true;
	else return false;
}

/* Waits till every child is zombified and reaps them all. 
*/
void reapChildrenSurely(){
	int pidDel = wait(NULL);

	/* waitpid(-1, NULL, 0)
	returns -1 : when no child process
	returns  0 : when no zombie children but yes child processes
	returns pid: when some zombie processes is reaped */

	while(pidDel > 0){
		if(backProcesses.find(pidDel)!= backProcesses.end()){
			backProcesses.erase(pidDel);
			printf("Shell : Background process finished\n");
		} 
		// printf("BACKG. CHILD PROCESS REAPED : %d\n", pidDel);
		pidDel = wait(NULL);
	}
	return;
};


/* Reaps all current zombie children and prints their PIDs.
*
*/
void reapChildren(){

	int pidDel = waitpid(-1, NULL, WNOHANG);
	while(pidDel > 0){
		if(backProcesses.find(pidDel)!= backProcesses.end()){
			backProcesses.erase(pidDel);
			printf("Shell : Background process finished\n");
		} 
		// printf("BACKG. CHILD PROCESS REAPED : %d\n", pidDel);
		pidDel = waitpid(-1, NULL, WNOHANG);
	}

	return;
}


/* Splits the string by space and returns the array of tokens
*
*/
char ** tokenize(char *line){
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	int i, tokenIndex = 0, tokenNo = 0;

	for(i = 0; i < strlen(line); i++){

		char readChar = line[i];

		if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
			token[tokenIndex] = '\0';
			if (tokenIndex != 0){
				tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
				strcpy(tokens[tokenNo++], token);
				// reset token
				tokenIndex = 0; 
			}
		} else {
			token[tokenIndex++] = readChar;
		}
	}

	free(token);
	tokens[tokenNo] = NULL; // NUll termination for a char* array
	return tokens;
}


/* Splits the string by & / && / &&& and returns the array of commands  
*
*/
char ** breakIntoCommands(char* line){
	char **commands = (char **)malloc(MAX_NUM_COMMANDS * sizeof(char *));
	char *command = (char *)malloc(MAX_INPUT_SIZE * sizeof(char));
	int i, commandIndex = 0, commandNo = 0;

	line[strlen(line)] = '&'; //acting as a ending delimiter
	// line[strlen(line)] = '&';

	for(i = 0; i < strlen(line); i++){

		char readChar = line[i];

		if (readChar == '&'){
			command[commandIndex] = '\0';
			if (commandIndex != 0){
				commands[commandNo] = (char*)malloc(MAX_INPUT_SIZE*sizeof(char));
				strcpy(commands[commandNo++], command);
				// reset token
				commandIndex = 0; 
			}
		} else {
			command[commandIndex++] = readChar;
		}
	}

	free(command);
	commands[commandNo] = NULL; // NUll termination for a char* array
	line[strlen(line)-1] = '\0'; //correcting our mistake
	return commands;
}

/* Cleans up the (char**)tokens structure  
*
*/
void freeTokens(char ** tokens){

	// Freeing the allocated memory	
	for(int i=0;tokens[i]!=NULL;i++){
		free(tokens[i]);
	}
	free(tokens);
	// printf("Process [%d] TOKENS freed\n", getpid());
	return;
}

/* Frees the (char **)commands structure memory 
*
*/
void freeCommands(char ** commands){
	// Freeing the allocated memory
	for(int i = 0; commands[i]!=NULL; i++){
		free(commands[i]);
	}
	free(commands);
	return;
}

/* -blocking-  Executes a command  specified via tokens list and cleans up the tokensList
*
*/
int execSingleCommandForeSerial(char **tokens, char ** commands){

	/* Check for cd commnand... */

	/* No input given */
	if(tokens[0] == NULL) return -1;

	/* If exit was given */
	if(isExitCmd(tokens)) {
		exitCall(tokens, commands);
		return 0;
	}

	if(tokens[0][0] =='c' && tokens[0][1] == 'd' && tokens[0][2]== '\0'){
		// printf("\nCD command called\n");
		int n;
		if(tokens[1]!=NULL && tokens[2]!=NULL){
			printf("Shell : Incorrect command : too many arguments\n");
			// continue;
		}
		else if((n = chdir(tokens[1])) < 0){ //works for "cd" too!
			perror("Shell : Incorrect command ");
		}
		
		freeTokens(tokens);

		return 0;
	}

	/* End cd check */
	
	
	int cpid = fork();

	if(cpid < 0){
		printf("Failed child creation\n");
		return -1;
	} else if(cpid == 0){
		// printf("\nChild process created:\n");
		// printf("CHILD OUTPUT------------\n\n");
		int n;
		if((n = execvp(tokens[0], tokens)) < 0){
			printf("Shell : Command \"%s\" not found\n", tokens[0]);
			freeTokens(tokens);
			freeCommands(commands);
			exit(-1); //necessary, otherwise wait(NULL) not working
			/* I think, the child will also go into infty loop, without exit */
		}
	}
	else{
		// printf("FOREG. CHILD PROCESS CREATED : %d\n", cpid);
		int n = waitpid(cpid, NULL, 0); /* wait(&status) === waitpid(-1, &status, 0) */
		// printf("FOREG. CHILD PROCESS REAPED : %d\n", n);
		// printf("\nCHILD OUTPUT DONE------------\n");
		// printf("\nChild terminated, lets continue\n");
	}
	freeTokens(tokens);
	return cpid;
}

/* -NON BLOCKING- Executes a command  specified via tokens list and cleans up the tokensList
*/
int execMultiCommandForeParallel(char **tokens, char ** commands){

	/* Check for cd commnand... */

	/* No input given */
	if(tokens[0] == NULL) return -1;

	/* If exit was given */
	if(isExitCmd(tokens)) {
		exitCall(tokens, commands);
		return 0;
	}


	if(tokens[0][0] =='c' && tokens[0][1] == 'd' && tokens[0][2]== '\0'){
		// printf("\nCD command called\n");
		int n;
		if(tokens[1]!=NULL && tokens[2]!=NULL){
			printf("Shell : Incorrect command : too many arguments\n");
			// continue;
		}
		else if((n = chdir(tokens[1])) < 0){ //works for "cd" too!
			perror("Shell : Incorrect command ");
		}
		
		freeTokens(tokens);

		return 0;
	}

	/* End cd check */
	
	
	int cpid = fork();

	if(cpid < 0){
		printf("Failed child creation\n");
		return -1;
	} else if(cpid == 0){
		// printf("\nChild process created:\n");
		// printf("CHILD OUTPUT------------\n\n");
		int n;
		if((n = execvp(tokens[0], tokens)) < 0){
			printf("Shell : Command \"%s\" not found\n", tokens[0]);
			freeTokens(tokens);
			freeCommands(commands);
			exit(-1); //necessary, otherwise wait(NULL) not working
			/* I think, the child will also go into infty loop, without exit */
		}
	}
	else{
		// printf("FORE. CHILD PROCESS CREATED : %d\n", cpid);
		// int n = waitpid(cpid, NULL, 0); /* wait(&status) === waitpid(-1, &status, 0) */
		// printf("\nCHILD OUTPUT DONE------------\n");
		// printf("\nChild terminated, lets continue\n");
	}
	freeTokens(tokens);
	return cpid;
}

/* NON BLOCKING - Executes a command on background, and cleans tokens
* Changes the PGID of background process
*/
int execSingleCommandBack(char **tokens, char ** commands){


	/* No input given */
	if(tokens[0] == NULL) return -1;

	/* If exit was given */
	if(isExitCmd(tokens)) {
		exitCall(tokens, commands);
		return 0;
	}
	/* Check for cd commnand... */
	if(tokens[0][0] =='c' && tokens[0][1] == 'd' && tokens[0][2]== '\0'){
		// printf("\nCD command called\n");
		int n;
		if(tokens[1]!=NULL && tokens[2]!=NULL){
			printf("Shell : Incorrect command : too many arguments\n");
			// continue;
		}
		else if((n = chdir(tokens[1])) < 0){ //works for "cd" too!
			perror("Shell : Incorrect command ");
		}
		
		freeTokens(tokens);

		return 0;
	}

	/* End cd check */
	
	
	int cpid = fork();

	if(cpid < 0){
		printf("Failed child creation\n");
		return -1;
	} else if(cpid == 0){
		// printf("\nChild process created:\n");
		// printf("CHILD OUTPUT------------\n\n");
		int n;
		
		setpgid(getpid(), getpid()); //IMP
		
		if((n = execvp(tokens[0], tokens)) < 0){
			printf("Shell : Command \"%s\" not found\n", tokens[0]);
			freeTokens(tokens);
			freeCommands(commands);
			exit(-1); //necessary, otherwise wait(NULL) not working
			/* I think, the child will also go into infty loop, without exit */
		}
	}
	else{
		backProcesses.insert(cpid);
		// printf("BACKG. CHILD PROCESS CREATED : %d\n", cpid);
		// int n = waitpid(cpid, NULL, 0); /* wait(&status) === waitpid(-1, &status, 0) */
		// printf("\nCHILD OUTPUT DONE------------\n");
		// printf("\nChild terminated, lets continue\n");
	}
	freeTokens(tokens);
	return cpid;
}

/* The various possible states for the input command. 
*/
enum commandState{
	singleBackground, 
	multiForeSerial, 
	multiForeParallel,
	junkie
};

/* Returns the commandState for the line inputed 
*/
enum commandState setLineState(char line[MAX_INPUT_SIZE]){

	enum commandState lineState = multiForeSerial;
	for(int i = 0; i<strlen(line); i++){
		if(line[i]=='&'){
			if(line[i+1]!='&')
				lineState = singleBackground;
			else if(line[i+2]!='&')
				lineState = multiForeSerial;
			else if(line[i+3]!='&')
				lineState = multiForeParallel;
			else{
				// printf("Not a valid & command. Treating as single command\n");
				lineState = junkie;
			}
			break;
		}
	}
	
	return lineState;
}


int main(int argc, char* argv[]) {
	signal(SIGTERM, sigterm_handler);
	signal(SIGINT, sigint_handler);
	char  line[MAX_INPUT_SIZE];    
	char  curDir[100];        
	char  **tokens = NULL;      
	char  **commands = NULL;  
	int i;
	char command[MAX_INPUT_SIZE];

	backProcesses.clear(); //initialization


	enum commandState lineState = multiForeSerial; //default

	// start the shell
	while(1) {
	
		carrySerialCommands = true;
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line)); // puts '\0' everywhere!!!
		// getcwd(curDir, 100); //get cur working dir
		printf("\033[0;33m");
		printf("\n $ ");
		// printf("\n%s $ ", curDir);
		printf("\033[0m");
		
		scanf("%[^\n]", line); /* common idiom to read the whole line till EOL */
		getchar();
		/* END: TAKING INPUT */

		reapChildren(); //good habit

		line[strlen(line)] = '\n';    // terminate with new line
		if(strlen(line)==1) continue; // to handle empty inputs

		commands = breakIntoCommands(line);

		/* Decide the lineState */
		/* Whether singleBackgnd, multiForeSerial, multiForeParallel */
		lineState = setLineState(line);
		// printf("lineState is : %d\n", lineState);

		if(lineState == junkie){
			printf("Shell : Incorrect usage of '&' delimiter\n");
			continue; //next user input
		}
		/* Decided on the lineState */


		if(lineState == multiForeSerial){
			for(i = 0; commands[i]!=NULL; i++){ //for Part-A, Part-B-B
				if(!carrySerialCommands) break; // when SIGINT is given
				strcpy(command, commands[i]);
				// /* Just print the commands */
				// printf("---------------------THe commands are : ");
				// printf("%s\n\n", command);
				tokens = tokenize(command);
				execSingleCommandForeSerial(tokens, commands); //Part-A
				reapChildren();
			}
		}

		if(lineState == singleBackground){
			strcpy(command, commands[0]);
			// /* Just print the commands */
			// printf("---------------------THe commands are : ");
			// printf("%s\n\n", command);
			tokens = tokenize(command);
			execSingleCommandBack(tokens, commands); //Part-A

		}

		if(lineState == multiForeParallel){
			// here make sure to reap the specific processes...
			// save the PIDs 
			int pids[MAX_NUM_COMMANDS];
			int lenPids = 0;
			int pid = 0;
			for(i = 0; commands[i]!=NULL; i++){ //for Part-A, Part-B-B
				if(!carrySerialCommands) break; // when SIGINT is given. stop
				strcpy(command, commands[i]);
				// /* Just print the commands */
				// printf("---------------------THe commands are : ");
				// printf("%s\n\n", command);
				tokens = tokenize(command);
				pid = execMultiCommandForeParallel(tokens, commands); //Part-A
				if(pid > 0){
					pids[lenPids++] = pid;
				}
			}

			// reap the PIDs stored!
			for(int i = 0; i<lenPids; i++){
				waitpid(pids[i], NULL, 0);
				// printf("FOREG. CHILD PROCESS REAPED : %d\n", pids[i]);
			}

		}


		freeCommands(commands);
		reapChildren();
		continue;

	}


	return 0;
}
