#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<sys/wait.h>
#include<string.h>


int main(int argc, char const *argv[])
{
    /* code */
    int pid = fork();
    // int pid2 = fork();


    if(pid < 0){
        printf("Process PID: %d ----Forking failed. \n", getpid());
    }
    else if(pid==0){
        printf("\nProcess PID: %d ---- This is child process speaking\n", getpid());
        char* sysarg[3];
        sysarg[0] = strdup("ls");
        sysarg[1] = strdup("-al");
        sysarg[2] = NULL;
        execvp(sysarg[0], sysarg);
    }
    else{
        wait(NULL);
        printf("\nProcess PID: %d ----This is parent process speaking\n", getpid());
        printf("\nProcess PID: %d ----My child is %d\n", getpid(), pid);
    }

    printf("Process PID: %d ----Exiting the process. My parent is %d\n ",getpid(), getppid());


    return 0;
}
