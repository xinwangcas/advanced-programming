////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Implementation for the Main method
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "math.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void defaultOutput()
{
		printf("\n");
		printf("#########################################################\n");
		printf("## 411 Homework: Numerical Analysis                    ##\n");
		printf("## Brian Chen, Oct 20, 2012                            ##\n");
		printf("#########################################################\n");
		printf("\n");
		printf("Usage:\n");
		printf("\n");
		printf("invert \n");
		printf("With no arguments, invert returns this usage statement.\n");
		printf("\n");
		printf("invert <filename>\n");
		printf("Prints out two matrix inverses for the provided matrix.\n");
		printf(" -- the first inverse is computed with simple inversion.\n");
		printf(" -- the second is computed with partial pivoting\n");
		printf("\n");
		printf("invert -format\n");
		printf("Describes the file format for the input file.\n");
		printf("\n");
		printf("#########################################################\n");
		printf("\n");
		system ("pause");
        exit(0);
}

void fileFormat()
{
		printf("\n");
		printf("#########################################################\n");
		printf("## 411 Homework: Numerical Analysis                    ##\n");
		printf("## Brian Chen, Oct 20, 2012                            ##\n");
		printf("#########################################################\n");
		printf("\n");
		printf("File format for a 3rd order matrix (input):\n");
		printf(" - numbers below are space deliminated.  no tabs.\n");
		printf(" - There are three lines, each with 3 numbers.\n");
		printf(" - They will be parsed as doubles.\n");
		printf(" - Comment lines start with #.  Comment lines cannot.\n");
		printf("   interrupt the 3 lines with the matrix, but can come.\n");
		printf("   before or after.\n");
		printf("\n");
		printf("---- Matrix file below, not including this line ----\n");
		printf("# comment\n");
		printf("# comment\n");
		printf("1.2 3.3 4.4\n");
		printf("1.112 2.24 0\n");
		printf("3.4 2.2 98.23452\n");
		printf("#\n");
		printf("#\n");
		printf("---- Matrix file above, not including this line ----\n");
		printf("\n");
		printf("#########################################################\n");
		printf("\n");
		exit(0);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Inside the result, the result is to be stored as:
//i=0 i=1 i=2
//[0] [3] [6]  //j=0
//[1] [4] [7]  //j=1
//[2] [5] [8]  //j=2
//
//where the above 3x3 matrix is as you would write it on paper.
//

void swap (double a , double b)
{
       double t;
       t = a;
       a = b;
       b = t;
}

double ** simpleMatrixInversion( double ** matrix ){
	int i = 0;
	int j = 0;
	int m = 0;
//      printMatrix(matrix);


	//insert your code here.//
	double gaussian[3][6];
	for (i = 0 ; i <= 2 ; i ++)
	{
        for (j = 0 ; j <= 5 ; j ++)
        {
            if (j <= 2)
            {
                  gaussian[i][j]=matrix[j][i];
            }
            else if (j - 3 == i)
            {
                  gaussian[i][j] = 1;
            }
            else
                  gaussian[i][j] = 0;
        }
     }
    
    
	for ( j = 0 ; j <= 2 ; j ++ )
	{
        for ( i = j + 1 ; i <= 2 ; i ++ )
        {
            double ratio;
            if (gaussian[j][j] != 0)
            {
               ratio = gaussian[i][j]/gaussian[j][j];
            }
            else
            {
                int k;
                k = j;
                k += 1;
                while(k < 3 && gaussian[k][j] == 0)
                {k += 1;}
                if (k == 3)
                {
                      printf("The inverse matrix does not exist!");
                }
                else 
                {

                     for (m = 0 ; m <= 5 ; m ++ )      
                     {
                         swap(gaussian[k][m],gaussian[j][m]);
                     }
                }
            }
            for ( m = j ; m <= 5 ; m ++ )
            {
                gaussian[i][m] = gaussian[i][m] - gaussian[j][m]*ratio;
            }
        }
    }
    
    double temp;    
    for (j = 2 ; j > 0 ; j -- )
    {
        temp = gaussian[j][j];
        for (m = j ; m <= 5 ; m ++ )
        {
            gaussian[j][m] = gaussian[j][m]/temp;
        }
    
        for (i = 0 ; i < j ; i ++ )
        {
            double ratio;
            ratio = gaussian[i][j]/1;
            for (m = 0 ; m <= 5 ; m ++ )
            {
                gaussian[i][m] = gaussian[i][m] - (ratio*gaussian[j][m]);
            }
        }
    }
    
    temp = gaussian[0][0];
        for (m = 0 ; m <= 5 ; m ++ )
        {
            gaussian[0][m] = gaussian[0][m]/temp;
        }
        

	//declare and allocate the result
	double ** result = new double*[3];
	for(i = 0; i<3; i++){
		result[i] = new double[3];
	}
    for (i = 0 ; i<= 2 ; i ++ )
    {
        for  (j = 3 ; j <= 5 ; j ++ )
        {
             result[i][j-3]=gaussian[i][j];
        }
    }
	return result;
}






double ** partialPivotInversion( double ** matrix ){
	int i = 0;
	int j = 0;
	int m = 0;
	//insert your code here.//
	double gaussian[3][6];
	for (i = 0 ; i <= 2 ; i ++)
	{
        for (j = 0 ; j <= 5 ; j ++)
        {
            if (j <= 2)
            {
                  gaussian[i][j]=matrix[j][i];
            }
            else if (j - 3 == i)
            {
                  gaussian[i][j] = 1;
            }
            else
                  gaussian[i][j] = 0;
        }
     }
   
	for ( j = 0 ; j <= 2 ; j ++ )
	{
        double max = 0 ;
        int max_index = 0;
        for (i = j ; i <= 2 ; i ++ )
        {
            if (max < fabs(gaussian[i][j]))
            {
                    max = fabs(gaussian[i][j]);
                    max_index = i;
            }
            if(max_index != j)
            {
                     for (m = 0 ; m <= 5 ; m ++ )
                     {
                         swap(gaussian[j][m], gaussian[max_index][m]);
                     }
            }
        }
        
        for ( i = j + 1 ; i <= 2 ; i ++ )
        {
            double ratio;
            if (gaussian[j][j] != 0)
            {
               ratio = gaussian[i][j]/gaussian[j][j];
            }
            else
            {
                int k;
                k = j;
                k += 1;
                while(k < 3 && gaussian[k][j] == 0)
                {k += 1;}
                if (k == 3)
                {
                      printf("The inverse matrix does not exist!");
                }
                else 
                {
                     for (m = 0 ; m <= 5 ; m ++ )                  
                     {
                         swap(gaussian[k][m],gaussian[j][m]);
                     }
                }
            }
            
            for (m = j ; m <= 5 ; m ++ )
            {
                gaussian[i][m] = gaussian[i][m] - gaussian[j][m]*ratio;
            }
        }
    }
    
    double temp;
    for (j = 2 ; j > 0 ; j -- )
    {
        temp = gaussian[j][j];
        for (m = j ; m <= 5 ; m ++ )
        {
            gaussian[j][m] = gaussian[j][m]/temp;
        }
    
        for (i = 0 ; i < j ; i ++ )
        {
            double ratio;
            ratio = gaussian[i][j]/1;
            for (m = 0 ; m <= 5 ; m ++ )
            {
                gaussian[i][m] = gaussian[i][m] - (ratio*gaussian[j][m]);
            }
        }
    }
    
    temp = gaussian[0][0];
    for (m = 0 ; m <= 5 ; m ++ )
    {
        gaussian[0][m] = gaussian[0][m]/temp;
    }
  
   	//declare and allocate the result
	double ** result = new double*[3];
	for(i = 0; i<3; i++){
		result[i] = new double[3];
	}
    for (i = 0 ; i<= 2 ; i ++ )
    {
        for  (j = 3 ; j <= 5 ; j ++ )
        {
             result[i][j-3]=gaussian[i][j];
        }
    }
	return result;

}

// ###########   ### ###    ###   ######  ###    ########### #######   ###   ######### ####  #### ###  ###
// ### ### ###   ### ###    ###   ### ### ###    ### ### ### #######   ###   ######### ###    ### ###  ###
// ###  #  ###  ###   ###   ###   ###  ######    ###  #  ### ###       ###   ######### ####  #### ###  ###
// ###     ###  #########   ###   ###   #####    ###     ### ###       ###   ###   ### ########## ########
// ###     ###  ######### ####### ###    ####    ###     ### #######   ###   ###   ###  ########  ######## 
// ###     ###  ###   ### ####### ###     ###    ###     ### #######   ###   ###   ###   ######   ######  

//execution format : [executable name] [argument]
int main(int argc, char* argv[])
{
	///usage handling
	if( argc == 1 ){
		defaultOutput();
	}

	///file format handling
	if( argc == 2 && ( strcmp(argv[1], "-format")==0 ) ){
		fileFormat();
	}

	///invert
	if( argc == 2 && ( strcmp(argv[1], "-format")!=0 ) ){
		double ** matrix = parseMatrixFile( argv[1] );
		
		//compute the simple inverse and print it
		double ** simpleInverse = simpleMatrixInversion( matrix );
		printMatrix(simpleInverse);
		
		//compute the inverse with partial pivoting and print it
		double ** partialInverse = partialPivotInversion( matrix );
		printMatrix(partialInverse);
		
		//clean up
		for(int i = 0; i<3; i++){
			delete[](simpleInverse[i]);
			delete[](partialInverse[i]);
			delete[](matrix[i]);
		}
		delete[](simpleInverse);
		delete[](partialInverse);
		delete[](matrix);
		
	}

    system("pause");
	return 0;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////


     
