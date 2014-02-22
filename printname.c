#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main() {
	int len;
	char name[50], prompt[50];
	write(1, "Please give me a name: ", 23);
	len = read(0, name, 50);
	while (len) {
		printf("len:%d\n",len);
		write(1, "Hello, ", 7);
		printf("mm\n");
		write(1, name, len - 1);
		write(1, "!\n", 2);
		write(1, "Please give me a name: ", 23);
		len = read(0, name, 50);
	}
}
