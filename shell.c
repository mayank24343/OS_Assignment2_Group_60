#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

typedef struct{
	char command[1024];
	pid_t* pid;
	int pid_count;
	time_t start_time;
	double duration;
} Commands;

int length(int n) {
	//length of integer
    if (n == 0) {
        return 1;
    }
    int l = 0;
    n = abs(n);
    while (n > 0) {
        n /= 10;
        l++;
    }
    return l;
}

Commands history[100]; //array to store all commands used
int count=0; //number of commands given

void show_history(){
	//show history with S.No and Command
	int c=length(count);
	printf("I am displaying the history\n");
	for (int i = 0; i < count; i++){
		printf("%*s",c-length(i)," ");
		printf("%d   %s\n", i+1, history[i].command);
	}
}

void cleanup_history(){
	//cleanup history pid mallocs
	for (int i = 0; i < count; i++){
		if (history[i].pid != NULL){
			free(history[i].pid);
		}
	}
}

void display_info(){
	int c=length(count);
	if (c>3){
		printf("%*s", c-3,"");
	}
	printf("PID   Start_time                 Duration   Commmand\n");
	for (int i = 0; i < count; i++){
		int l=length(i);
		if (l<3 && c<3){
			printf("%*s",3-l," ");
		}
		else if (c>3){
			printf("%*s",c-l," ");
		}
		char *t = ctime(&history[i].start_time);
                t[strcspn(t, "\n")] = '\0';
		for (int j = 0; j < history[i].pid_count; j++){
			printf("%d ",history[i].pid[j]);
		}
		printf("   %s   %f   %s\n", t, history[i].duration, history[i].command);
	}
	//function to display child process pid, the time they were starting to execute and the duration for execution etc. 
}

char** read_command(char* command){
	//take string command and return tokens (space separated words)
	char** args = malloc(sizeof(char*)*64);
	if (args == NULL){
		perror("Could not allocate memory for args");
		exit(0);
	}
	int i = 0;

	char* token = strtok(command," \n");
	while (token != NULL && i < 63){
		args[i] = token;
		i++;
		token = strtok(NULL," \n");
	}
	args[i] = NULL;//execvp takes NULL terminated

	return args;
}

int create_process_and_run(char* command){
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
		if (strcmp(args[0],"cd") == 0){
			char* path;
			if (args[1] == NULL){
				path = getenv("HOME");
			}
			else{
				path = args[1];
			}
			if (chdir (path)!= 0){
				perror("cd");
			}
			history[count].pid = malloc(sizeof(pid_t));
			history[count].pid[0] = getpid();
			history[count].start_time = time(NULL);
			history[count].duration = 0.0;
			return 1;
		}
		time_t start_time = time(NULL);
		pid_t pid = fork();
		if (pid < 0){
			perror("Could not create child process");
			exit(0);
		}
		else if (pid == 0){
			//in the child process
			if (strcmp(args[0],"history") == 0){
				//need special handling for this
				show_history();
				exit(0);
			}
			else{
				execvp(args[0], args);
				perror("This should never have executed!!");
				exit(0);
			}
		}
		else{	
			history[count].pid = malloc(sizeof(pid_t));
			history[count].pid_count = 1;
			history[count].pid[0]=pid;
			history[count].start_time=start_time;
			wait(NULL);
			time_t end_time=time(NULL);
			history[count].duration=difftime(end_time, start_time);
		}

	}
	else{
		//pipes exist
		//write on 1, read from  0
		int fd[(i-1)*2]; //i-1 pipes for i commands/processes
		for (int j = 0; j < i-1; j++){
			if (pipe(fd+j*2) < 0){
				perror("pipe didnt work!");
				exit(0);
			}
		}
		int pids[i];
		time_t start_time = time(NULL);
		
		for (int p_no = 0; p_no < i; p_no++){
			int child = fork();
			if (child < 0){
				perror("Could not create child!");
				exit(0);
			}
			else if (child == 0){
				if (p_no > 0){
					dup2(fd[(p_no-1)*2],STDIN_FILENO);
				}
				if (p_no < i-1){
					dup2(fd[p_no*2+1],STDOUT_FILENO);
				}
				for (int j = 0; j < i-1; j++){
					close(fd[j*2]);
					close(fd[j*2+1]);
				}
				char** args = read_command(commands[p_no]);
				execvp(args[0],args);
				perror("child not executing the command using execvp!");
				exit(0);
			}
			else{
				pids[p_no] = child;
			}
		}
		for (int j = 0; j < i-1; j++){
			close(fd[j*2]);
			close(fd[j*2+1]);
		}
		for (int j = 0; j < i; j++){
			waitpid(pids[j],NULL,0);
		}

		time_t end_time = time(NULL);
		history[count].pid = malloc(i*sizeof(pid_t));
		for (int j = 0; j < i; j++){
			history[count].pid[j] = pids[j];
		}
		history[count].pid_count = i;
		history[count].start_time = start_time;
		history[count].duration = difftime(end_time,start_time);
		status = 1;
	}

	free(commands);
	free(token);
	return 1;
}

int launch(char* command){
	//run command
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
		if (strcmp(command,"exit\n") == 0){
			display_info();
			//exit the shell
			return;
		}
		strcpy(history[count].command, command);//store command in history
		status = launch(command);//launch command
		count++;
		free(command); //free memory for cleanup
	} while (status);
}

int main(){
	shell_loop();
	cleanup_history();
	return 0;
}
