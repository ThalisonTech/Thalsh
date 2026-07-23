#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

struct Builtin {
	char* name;
	int (*program)(char** args);
}

extern int should_run;
extern int exit_status;
extern struct Builtin builtins[];

int builtin_cd(char** args);
int builtin_export(char** args);
int builtin_unset(char** args);
int builtin_exit(char** args);
