#include  <stdio.h>
#include  <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

/* Splits the string by space and returns the array of tokens
*
*/
char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

	char readChar = line[i];

	if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
	  token[tokenIndex] = '\0';
	  if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
	  }
	} else {
	  token[tokenIndex++] = readChar;
	}
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}


int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              


	while(1) {			
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		printf("\033[0;33m");
		printf("$$$$$$$$ ");
		printf("\033[0m");

		scanf("%[^\n]", line);
		getchar();

		printf("Command entered: %s (remove this debug output later)\n", line);
		/* END: TAKING INPUT */

		int j=0;
		int nof2=1,nof3=1;
		char **lines=(char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
		int tp=0; // type of input
		lines[0]=(char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
		for(int i=0;i<strlen(line);i++){
			if(line[i]=='&'){
				if(line[i+1]=='&'){
					if(line[i+2]=='&'){					//this implies there is '&&&' in the command
						tp=3;
						i=i+2;
						nof3++;
						lines[nof3-1]=(char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
						j=0;
					}
					else{								//this implies there is '&&' in the command
						tp=2;
						i++;
						nof2++;
						lines[nof2-1]=(char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
						j=0;
					}
				}
				else{									//if only 1 '&' is present
					tp=1;
				}
			}
			else{										//if the char read is not '&'
				if(tp==0){								//if there is no '&' in the command yet
					lines[0][j]=line[i];
					j++;
				}
				else if(tp==2){							//if a '&&' is read from the command
					lines[nof2-1][j]=line[i];
					j++;
				}
				else if(tp==3){							//if a '&&&' is read from the command
					lines[nof3-1][j]=line[i];
					j++;
				}
			}
		}
		char lu[MAX_INPUT_SIZE];
		printf("tp=%d\n",tp);
		if(tp==0){										//if no '&' in the command line
			line[strlen(line)]='\n';
			tokens = tokenize(line);
			if(tokens[0][0]=='c' && tokens[0][1]=='d' && tokens[0][2]=='\0'){		// if command is "cd xxxx"
				if(tokens[2]!=NULL){
					printf(" Shell : Incorrect command\n");
				}
				else{
					if(chdir(tokens[1])<0){
						printf(" Shell : Incorrect command\n");
					}
				}
			}
			else{										//if command is not "cd xxxx"
				int cid=fork();
				if(cid<0){
					printf("ERROR: Child process could NOT be created\n");
					exit(1);
				}
				else if(cid==0){
					execvp(tokens[0],tokens);			//execute the command
					printf("ERROR: Execution Failed!!\n");
				}
				else{
					printf("Child created\n");
					int wc=waitpid(cid,NULL,0);			//wait for the created child to be reaped
					printf("Child reaped\n");
				}
			}
		}
		else if(tp==1){									//if 1 '&' in the command line
			strcpy(lu,lines[0]);
			lu[strlen(lu)]='\n';
			tokens=tokenize(lu);
			if(tokens[0][0]=='c' && tokens[0][1]=='d' && tokens[0][2]=='\0'){		// if command is "cd xxxx"
				if(tokens[2]!=NULL){
					printf(" Shell : Incorrect command\n");
				}
				else{
					if(chdir(tokens[1])<0){
						printf(" Shell : Incorrect command\n");
					}
				}
			}
			else{
				int cid=fork();
				if(cid<0){
					printf("ERROR: Child process could NOT be created\n");
					exit(1);
				}
				else if(cid==0){
					execvp(tokens[0],tokens);			//execute the command
					printf("ERROR: Execution Failed!!\n");
				}
				else{
					printf("Child created\n");			//no wait here cause it's abackground process
				}
			}
		}
		else if(tp==2){									//if 2 '&'s in the command line
			for(int i=0;i<nof2;i++){					//for each command separated by "&&"
				strcpy(lu,lines[i]);
				lu[strlen(lu)]='\n';
				tokens=tokenize(lu);
				if(tokens[0][0]=='c' && tokens[0][1]=='d' && tokens[0][2]=='\0'){	// if command is "cd xxxx"
					if(tokens[2]!=NULL){
						printf(" Shell : Incorrect command\n");
					}
					else{
						if(chdir(tokens[1])<0){
							printf(" Shell : Incorrect command\n");
						}
					}
				}
				else{									//if command is not "cd xxxx"
					int cid=fork();
					if(cid<0){
						printf("ERROR: Child process could NOT be created\n");
						exit(1);
					}
					else if(cid==0){
						execvp(tokens[0],tokens);			//execute the command
						printf("ERROR: Execution Failed!!\n");
					}
					else{
						printf("Child created\n");
						int wc=waitpid(cid,NULL,0);		//wait for the created child to be reaped
						printf("Child reaped\n");
					}
				}
			}
		}
		else if(tp==3){									//if 3 '&'s in the command line
			int chid[MAX_TOKEN_SIZE];
			int length=0;
			printf("nof3=%d\n",nof3);
			for(int i=0;i<nof3;i++){					//for each command separated by "&&&"
				strcpy(lu,lines[i]);
				lu[strlen(lu)]='\n';
				tokens=tokenize(lu);
				if(tokens[0][0]=='c' && tokens[0][1]=='d' && tokens[0][2]=='\0'){		// if command is "cd xxxx"
					if(tokens[2]!=NULL){
						printf(" Shell : Incorrect command\n");
					}
					else{
						if(chdir(tokens[1])<0){
							printf(" Shell : Incorrect command\n");
						}
					}
				}
				else{									//if command is not "cd xxxx"
					int cid=fork();
					if(cid<0){
						printf("ERROR: Child process could NOT be created\n");
						exit(1);
			,lengt		}
					else if(cid==0){
						execvp(tokens[0],tokens);		//execute the command
						printf("ERROR: Execution Failed!!\n");
						exit(-1);
					}
					else{
						printf("Child %d,length=%d created\n",cid,length);
						chid[length]=cid;				//saving the pid of the child process
						length++;
					}
				}
			}
			for(int i=0;i<length;i++){
				int wc=waitpid(chid[i],NULL,0);         //wait for each child with pid=chid[i] created to be reaped 
				printf("Child %d reaped\n",chid[i]);
			}
		}

		int wt=waitpid(-1, NULL, WNOHANG);				//this wait function is for background processes
	   
		// Freeing the allocated memory	
		for(int i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);

	}
	return 0;
}
