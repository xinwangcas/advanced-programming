#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
int main() {
	int status;
	char* myargv[2]; // placement of * vs. space d/n matter
	if (fork() == 0) {
		myargv[0] = calloc(strlen("hello") + 1, sizeof(char));
		strcpy(myargv[0], "hello");
		myargv[1] = 0;
		execv("hello", myargv);
		printf("Uh oh, execve didn't work!\n");
		exit(-1);
	}
	else // do we need an ‘else’ here?
		waitpid(-1, &status, 0);
}
