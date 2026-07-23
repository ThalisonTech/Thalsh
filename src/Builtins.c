#include <Thalsh.h>

static size_t arglen(char** args){
	int i = 0;
	while (args[i] != NULL) i++;
	return i;
}

int builtin_cd(char** args){
	int argc = arglen(args);
	if (argc > 2){
		fprintf(stderr, "thalsh: cd: too many arguments\n");
		return 2;
	} else if (argc == 1){
		if (chdir(getenv("HOME")) != 0){
			fprintf(stderr, "thalsh: cd: %s: %s\n", getenv("HOME"), strerror(errno));
			return 1;
		}
	} else {
		if (chdir(args[1]) != 0){
			fprintf(stderr, "thalsh: cd: %s: %s\n", args[1], strerror(errno));
			return 1;
		}
	}

	return 0;
}

int builtin_exit(char** args){
	int argc = arglen(args);
	if (argc > 2){
		fprintf(stderr, "thalsh: exit: too many arguments\n");
		return 2;
	} else if (argc == 1){
		should_run = 0;
	} else {
		should_run = 0;
		exit_from_thalsh(NULL, NULL, NULL);
		return atoi(args[1]);
	}

	exit_from_thalsh(NULL, NULL, NULL);
	return exit_status;
}

struct Builtin builtins[] = {
	{"cd", builtin_cd},
	{"exit", builtin_exit},
	{NULL, NULL}
};
