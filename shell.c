#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

void show_history(){
	printf("I am displaying the history\n");
}

void display_info(){
	//function to display child process pid, the time they were starting to execute and the duration for execution etc. 
}

char** read_command(char* command){
	char** args = malloc(sizeof(char*)*64);
	int i = 0; 

	char* token = strtok(command," \n");
	while (token != NULL && i < 63){
		args[i] = token;
		i++;
		token = strtok(NULL," \n");
	}
	args[i] = NULL;

	return args;	
}

int create_process_and_run(char* command){
	if (strcmp(command,"exit\n") == 0){
		//exit the shell
		return 0;
	}

	//to find number of commands (pipe separated)
	char* commands[16];
	int i = 0; 
	char* token = strtok(command,"|\n");
	while (token!= NULL && i < 16){
		commands[i] = token;
		i++;
		token = strtok(NULL,"|\n");
	}
	
	int status = 0;
	if (i == 1){
		//means no pipe, single command
		char** args = read_command(commands[0]);
		status = fork();
		if (status < 0){
			printf("Could not create child process");
		}
		else if (status == 0){
			//in the child process
			if (strcmp(args[0],"cd") == 0){
				//need special handling for cd
			}
			else if (strcmp(args[0],"history") == 0){
				//need special handling for this
				show_history();
				exit(0);
			}
			else{
				execvp(args[0], args);
				exit(0);
			}
		}
		
	}
	else{
		printf("pipes exist!!!!!!!\n");
		//pipes exist
		int fd[i*2];
		for (int j = 0; j < i; j++){
			pipe(fd+j*2);
		}
		for (int j = 0; j <= i; j++){
			status = fork();
			if (status < 0){
				printf("Error occurred\n");
			}
			else if (status == 0){
				if (j > 0){
					dup2(fd[(j-1)*2],STDIN_FILENO);
				}
				if (j < i){
					dup2(fd[j*2+1],STDOUT_FILENO);
				}
			}
			for(int j = 0; j < 2*i; j++){
				close(fd[j]);
			}
			
			char** args = read_command(commands[j]);
			execvp(args[0], args);

		}
		
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

