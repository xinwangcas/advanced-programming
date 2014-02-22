/* 
 * File:   main.c
 * Author: xiw412
 *
 * Created on September 13, 2012, 12:02 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/*
 * 
 */
extern int optind;
extern int opterr;
extern int optopt;

char file_char; //characters in input file
int c_flag = 0; //read options of wc command
int C_m_flag = 0;
int l_flag = 0;
int w_flag = 0;
int l_count = 0;
int w_count = 0;
int c_count = 0;
int C_m_count = 0;
int command_append; //options of command wc
int l_count_sum = 0;
int w_count_sum = 0;
int c_count_sum = 0;
int C_m_count_sum = 0;

int WCount (FILE * fp )
{
	while ((file_char = getc(fp))!=EOF )
	{
		if (!iswspace(file_char))
		{
			while (!iswspace (file_char))
			{
				c_count ++ ;
				C_m_count ++;
				file_char = getc(fp);
			}
			w_count ++ ;
		}
		if (file_char == '\n')
		{
			l_count++;
		}
		c_count ++ ;
		C_m_count ++ ;
	}
}

int PrintOut ( char * FileName, int IfTotal )
{
//	int i;
//	for ( i = 1 ; i <= FileNumber ; i ++ )
//	{
	
		if ( l_flag )
		{
			printf ("%8d", IfTotal ? l_count_sum : l_count);
		}
		if ( w_flag )
		{
			printf ("%8d", IfTotal ? w_count_sum : w_count );
		}
		if ( c_flag )
		{
			printf ("%8d", IfTotal ? c_count_sum : c_count );
		}
		if ( C_m_flag )
		{
			printf ("%8d", IfTotal ? C_m_count_sum : C_m_count );
		}
		printf (" %s\n", FileName);
//	}
	
}

int main(int argc, char** argv)
{    
    while ((command_append = getopt ( argc, argv, "lwcmC"))!= -1)
    {
//	    printf ("optind:%d opterr:%d optopt:%d getopt:%c\n", optind, opterr, optopt, command_append);
        switch ( command_append )
        {
            case 'l':
                l_flag = 1;
		break;
            case 'w':
                w_flag = 1;
		break;
            case 'c':
                c_flag = 1;
		if ( C_m_flag )
		{
			printf ("usage: wc [-c | -m | -C] [-lw] [file ...]\n");
			return 1;
		}
		break;
            case 'C':
	    case 'm':
                C_m_flag = 1;
		if ( c_flag )
		{
			printf ("usage: wc [-c | -m | -C] [-lw] [file ...]\n");
			return 1;
		}
		break;
            default:
		printf ("usage: wc [-c | -m | -C] [-lw] [file ...]\n");
		return 1;
        }
    }

//    printf("l_flag: %d, w_flag: %d, c_flag: %d, C_m_flag: %d\n", l_flag, w_flag, c_flag, C_m_flag);
    
    if (optind == 1)  //Default option -lwc
    {
	    l_flag = 1;
	    w_flag = 1;
	    c_flag = 1;
    }

//    printf("l_flag: %d, w_flag: %d, c_flag: %d, C_m_flag: %d\n", l_flag, w_flag, c_flag, C_m_flag);

    /**********************************************************************************************/

    if ( optind < argc ) //count from a file, otherwise, count from standard input
    {
	    int i;
	    FILE * fp;
	    for ( i = optind ; i < argc ; i ++ )
	    {
		   l_count = 0;
		   w_count = 0;
		   c_count = 0;
		   C_m_count = 0;
		   fp = fopen (argv[i],"r");
		   if (fp == NULL)
			   printf("wc: cannot open %s\n",argv[i]);
		   else
		   {
			   WCount (fp);
			   PrintOut (argv[i],0);
		   	   l_count_sum = l_count + l_count_sum;
		  	   w_count_sum = w_count + w_count_sum;
		  	   c_count_sum = c_count + c_count_sum;
		  	   C_m_count_sum = C_m_count + C_m_count_sum;
		   }

	    }
	    if ( (argc - optind ) > 1 )
		{
			PrintOut("total",1 );
		}


    }

    else if (optind == argc)
    {
	WCount (stdin);
	if ( l_flag )
	{
		printf ("%8d", l_count);
	}
	if ( w_flag )
	{
		printf ("%8d", w_count);
	}
	if ( c_flag )
	{
		printf ("%8d", c_count);
	}
	if ( C_m_flag )
	{
		printf ("%8d", C_m_count);
	}
	printf ("\n");

    }
//    printf("l_count: %d, w_count: %d, c_count: %d, C_m_count: %d\n", l_count, w_count, c_count, C_m_count);
    return (EXIT_SUCCESS);
}

