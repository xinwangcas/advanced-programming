#include <stdio.h>
#include <stdlib.h>

int main()
{
	char c;
	int count = 0;
	while ((c = getc(stdin)) != EOF)
	{
		if( (c == '\t')&&(count == 0))
		{
			putc(' ',stdout);
			count++;
		}
		else if ((c == ' ')&&(count == 0))
		{
			count++;
			putc(' ',stdout);
		}
		else if ((c != '\t')&&(c != ' '))
		{
			putc(c,stdout);
			count = 0;
		}
		else if(count >= 1)
		{
			;
		}
	}
	return 0;
}
