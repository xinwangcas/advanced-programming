#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

extern char* optarg;
extern int optind;
extern int opterr;
extern int optopt;

int main(int argc, char* argv[]){
	int i;
	int iters = 5;
	opterr = 0;
	int opt;
	while ((opt = getopt(argc, argv, "i:")) != -1)
	{
		switch(opt){
			case'i':
			//	printf("%s",optarg);
				iters = strtol(optarg,NULL,10);
				break;
		}
	}
	for (i = 0;i <= iters;i ++)
	{
		printf("Hello,Lehigh\n");
	}
	return 0;
}
