#include <Thalsh.h>

int should_run = 1;
int exit_status = 0;
volatile sig_atomic_t sigint_true = 0;

void exit_from_thalsh(char* input, char* command, char** args){
	while (wait(NULL) > 0);
	printf("logout\n");
	free(input);
	free(command);
	free(args);
	should_run = 0;
}

void sigint_handler(int signo){
	(void)signo;
	sigint_true = 1;
}

void mainloop(void){
	struct sigaction sa;
	sa.sa_handler = sigint_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);

	while (should_run){
		if (sigint_true == 1){
			printf("\n");
			clearerr(stdin);
			sigint_true = 0;
			continue;
		}

		printf("Thalsh> ");
		fflush(stdout);
		char* input = NULL;
		size_t input_size = 0;
		ssize_t getline_bytes = getline(&input, &input_size, stdin);
		if (getline_bytes == -1){
			if (feof(stdin)){
				printf("\n");
				exit_from_thalsh(NULL, NULL, NULL);
				continue;
			}

			perror("getline");
			exit_status = 1;
			exit_from_thalsh(NULL, NULL, NULL);
			continue;
		}

		input[strcspn(input, "\n")] = '\0';

		char* command = strdup(input);
		if (!command){
			perror("strdup");
			exit_status = 1;
			exit_from_thalsh(input, NULL, NULL);
			continue;
		}

		size_t capacity = 8; 
		char** args = malloc(sizeof(char*) * capacity);

		int i = 0;
		args[i] = strtok(command, " \t");
		while (args[i] != NULL){
			i++;

			if (i >= capacity){
				char** tmp_args = realloc(args, capacity * sizeof(char*) * 2);
				if (!tmp_args){
					perror("realloc");
					exit_status = 1;
					exit_from_thalsh(input, command, args);
					continue;
				}

			}
			
			args[i] = strtok(NULL, " \t");
		}

		if (args[0] == NULL){
			free(input);
			free(command);
			free(args);
			continue;
		}

		int builtin_executed = 0;
		for (int i = 0; builtins[i].name != NULL; i++){
			if (strcmp(builtins[i].name, args[0]) == 0){
				exit_status = builtins[i].program(args);
				builtin_executed = 1;
				break;
			}
		}

		if (builtin_executed){
			free(input);
			free(command);
			free(args);
			continue;
		}

		pid_t pid = fork();
		if (pid == 0){
			execvp(args[0], args);
			if (errno == ENOENT){
				if (strchr(args[0], '/')){
					fprintf(stderr, "thalsh: %s: %s\n", args[0], strerror(errno));
				} else {
					fprintf(stderr, "thalsh: %s: command not found\n", args[0]);
				}

				_exit(127);
			}

			fprintf(stderr, "thalsh: %s: %s\n", args[0], strerror(errno));
			_exit(126);
		} else if (pid == -1){
			perror("fork");
		} else if (pid > 0){
			int status;
			if (waitpid(pid, &status, 0) == -1) {
				perror("waitpid");
			} else {
				if (WIFEXITED(status)){
					exit_status = WEXITSTATUS(status);
				} else if (WIFSIGNALED(status)){
					int signal = WTERMSIG(status);
					if (signal == SIGSEGV){
						fprintf(stderr, "Segmentation fault");
						if (WCOREDUMP(status)){
							fprintf(stderr, " (core dumped)");
						}
						fprintf(stderr, "\t%s\n", input);
					}
				}
			}
		}

		free(input);
		free(command);
		free(args);
	}
}

int main(){
	mainloop();
	return exit_status;
}
