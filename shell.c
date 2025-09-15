#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int global = 0;

void show_history(){
	printf("I am displaying the history\n");
}

int create_process_and_run(char* command){
	if (strcmp(command,"exit\n")==0){
		return 0; //exit the shell
	}
	else if (strcmp(command,"history\n") == 0){
		//show history of commands received
		show_history();
	}


	int status = fork();
	if (status < 0){
		printf("Could not create child process!");
	}
	else if (status == 0){
		char *args[64];
		int i = 0;

		//loop to collect tokens from command separated by space or \n
		char *token = strtok(command, " \n");
		while (token != NULL && i < 63) {
			if (strcmp(token,"|") == 0){
				//pipes in input, do differently
				printf("detected pipe in this input.\n");
			}
			else if (strcmp(token,"cd") == 0){
				//cd detected, do differently
				printf("cd detected\n");
			}

			args[i++] = token;
			token = strtok(NULL," \n");
		}

		args[i] = NULL; // execvp requires NULL-terminated argv
		execvp(args[0], args);

	}

	wait(NULL);
	return status;
} 

int launch(char* command){
	int status;
	status = create_process_and_run(command);
	return status;
}

void shell_loop(){
	int status;
	do {
		printf("group60@os:~$");
		char* command = (char*)malloc(1024*sizeof(char)); //1KB input
		fgets(command,1024,stdin);
		status = launch(command);
	} while (status);
}

int main(){
	shell_loop();
	return 0;
}

