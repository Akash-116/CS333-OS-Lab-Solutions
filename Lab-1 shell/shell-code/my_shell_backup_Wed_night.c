#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define MAX_NUM_COMMANDS 64

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


/* Splits the string by &/&&/&&& and returns the array of commands  
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
	return commands;
}

/* Cleans up the tokens structure  
*
*/
void freeTokens(char ** tokens){

	// Freeing the allocated memory	
	for(int i=0;tokens[i]!=NULL;i++){
		free(tokens[i]);
	}
	free(tokens);
	return;
}

/* Frees the (char **) commands structure memory 
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

/* Executes a command specified via tokens list and cleans up the tokensList
*
*/
void execSingleCommandForeground(char **tokens){

		/* Check for cd commnand... */

		if(tokens[0][0] =='c' && tokens[0][1] == 'd' && tokens[0][2]== '\0'){
			// printf("\nCD command called\n");
			int n;
			if(tokens[1]!=NULL && tokens[2]!=NULL){
				printf("Shell : Incorrect command : too many arguments\n");
				// continue;
			}
			else if((n = chdir(tokens[1])) < 0){ //works for "cd" too!
				perror("Shell : Incorrect command");
			}
			
			freeTokens(tokens);

			return;
		}

		/* End cd check */
       
	   
		int cpid = fork();

		if(cpid < 0){
			printf("Failed child creation\n");
			return;
		} else if(cpid == 0){
			// printf("\nChild process created:\n");
			// printf("CHILD OUTPUT------------\n\n");
			int n;
			if((n = execvp(tokens[0], tokens)) < 0){
				printf("Shell : Command/Executable \"%s\" not found\n", tokens[0]);
				exit(-1); //necessary, otherwise wait(NULL) not working
				/* I think, the child will also go into infty loop, without exit */
			}
		}
		else{
			printf("CHILD PROCESS CREATED : %d\n", cpid);
			int n = wait(NULL);
			printf("CHILD PROCESS REAPED : %d\n", n);
			// printf("\nCHILD OUTPUT DONE------------\n");
			// printf("\nChild terminated, lets continue\n");
		}

	   
	   
		freeTokens(tokens);


	return;
}


int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];    
	char  curDir[100];        
	char  **tokens = NULL;      
	char  **commands = NULL;  
	int i;
	char command[MAX_INPUT_SIZE];

	enum commandState{
		singleBackground, 
		multiForeSerial, 
		multiForeParallel
	};

	enum commandState lineState = multiForeSerial; //default



	// start the shell
	while(1) {
	
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		getcwd(curDir, 100); //get cur working dir
		printf("\n%s $ ", curDir);
		// printf("\n $ ");
		scanf("%[^\n]", line); /* common idiom to read the whole line till EOL */
		getchar();
		/* END: TAKING INPUT */


		line[strlen(line)] = '\n';    // terminate with new line
		if(strlen(line)==1) continue; // handling a blank/empty line
		commands = breakIntoCommands(line);


		// tokens = tokenize(line);
   
    //    // here we just print them
	// 	for(i=0; tokens[i]!=NULL; i++){
	// 		printf("Tokens found: %s (remove this debug output later)\n", tokens[i]);
	// 	}
		for(i = 0; commands[i]!=NULL; i++){ //for Part-A, Part-B-B
			strcpy(command, commands[i]);

			/* Just print the commands */
			printf("---------------------THe commands are : ");
			printf("%s\n\n", command);

			
			tokens = tokenize(command);
			execSingleCommandForeground(tokens); //Part-A
		}
		freeCommands(commands);



	}
	return 0;
}
