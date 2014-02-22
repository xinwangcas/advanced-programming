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

void defaultOutput() //default output when there are no command line parameters
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
        exit(0);
}

void fileFormat()//Explain output format when the command line parameter is -format
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



/*
 *Author : Xin Wang
 *user:    xiw412   
 */

//define a swap function to exchange two numbers/rows/columns
void swap (double &a , double &b)
{
       double t;
       t = a;
       a = b;
       b = t;
}

//simple inversion algorithm for gaussian elimination

double ** simpleMatrixInversion( double ** matrix ){
//i,j and m are used for marking array subcripts i represents row, j represents column, m traces numbers in a row one by one.
	int i = 0;
	int j = 0;
	int m = 0;


	//insert your code here.//
	double gaussian[3][6];
//obtain a 3*6 array, the left part is the input matrix, the right part is a 3*3 identity matrix.
//when the left matrix becomes an identity one, the right matrix becomes the inverse matrix.
	for (i = 0 ; i <= 2 ; i ++)
	{
        	for (j = 0 ; j <= 2 ; j ++)
        	{
                	gaussian[i][j]=matrix[i][j];
			if(i == j)
				gaussian[i][j + 3] = 1;
			else
				gaussian[i][j + 3] = 0;
        	}
     	}
   
//eliminating coefficients of x1, x2, x3 in every column to get an upper triangular matrix
	
	for ( j = 0 ; j <= 2 ; j ++ )
	{
		int k;
		k = j;
		while(k < 3 && gaussian[k][j] == 0)
		{k += 1;}
		if (k == 3)//when coefficients of a certain column are all zeors, the inverse matrix does not exist
		{
		      printf("The inverse matrix does not exist!");
		}
		else if(k != j) 
		{
		     for (m = 0 ; m <= 5 ; m ++ )//when coefficient of x in a certain row is zero, swap it with a non-zero row                  
		     {
			 swap(gaussian[k][m],gaussian[j][m]);
		     }
	
		}

		assert(gaussian[j][j] != 0);
        //to make sure that no starting point of each elimination process is zero

		for ( i = j + 1 ; i <= 2 ; i ++ )//eliminate each row of the lower triangular matrix
		{
		    if(gaussian[i][j] == 0)
			    continue;
		    else
		    {
			    double ratio;
			    ratio = gaussian[i][j]/gaussian[j][j];
			    for (m = j ; m <= 5 ; m ++ )
			    {
				gaussian[i][m] = gaussian[i][m] - gaussian[j][m]*ratio;
			    }
		    }
	
		}
    }
    
 //perform back substitution from the last row of the upper triangular matrix and subtract x3,x2,x1 from each column
    double temp;
    for (j = 2 ; j >= 0 ; j -- )
    {
        temp = gaussian[j][j];
        for (m = j ; m <= 5 ; m ++ )
        {
            gaussian[j][m] = gaussian[j][m]/temp;
        }
    
	
        for (i = 0 ; i < j ; i ++ )
        {
            double ratio;
            ratio = gaussian[i][j]/1;//calculate the ratio to multiply a row and subtract it from the default row
            for (m = j ; m <= 5 ; m ++ )
            {
                gaussian[i][m] = gaussian[i][m] - (ratio*gaussian[j][m]);
            }
	
        }
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





//using partial pivoting method in gaussian elimination.

/*The only difference between partial pivoting and simple method is:
In partial pivoting, we pick the largest value xi as the new pivot.
Then we swap the new pivot and the whole row with the original row.      
*/

double ** partialPivotInversion( double ** matrix ){

	int i = 0;
	int j = 0;
	int m = 0;


	//insert your code here.//
 double gaussian[3][6];

//obtain a 3*6 array, the left part is the input matrix, the right part is a 3*3 identity matrix.
// when the left matrix becomes an identity one, the right matrix becomes the inverse matrix.

	for (i = 0 ; i <= 2 ; i ++)
	{
        	for (j = 0 ; j <= 2 ; j ++)
        	{
                	gaussian[i][j]=matrix[i][j];
			if(i == j)
				gaussian[i][j + 3] = 1;
			else
				gaussian[i][j + 3] = 0;
        	}
     	}
   
//In partial pivoting, we select the number with the bigest absolute magnitude as the pivot	
	for ( j = 0 ; j <= 2 ; j ++ )
	{
		double max = 0 ;//because we need to find the number with bigest absolute magnitude, we start from max = 0
		int max_index = 0;
		for (i = j ; i <= 2 ; i ++ )
		{
		    if (max < fabs(gaussian[i][j]))
		    {
			    max = fabs(gaussian[i][j]);
			    max_index = i;//find the index of the largest pivot
		    }
		    if(max_index != j)
		    {
			     for (m = 0 ; m <= 5 ; m ++ )
			     {
				 swap(gaussian[j][m], gaussian[max_index][m]);//swap the largest pivot with the default pivot
			     }
		    }
		}
		
//the rest of the algorithm is the same as the simple matrix inversion method
//eliminating coefficients of x1, x2, x3 in every column to get an upper triangular matrix
		int k;
		k = j;
		while(k < 3 && gaussian[k][j] == 0)
		{k += 1;}
		if (k == 3)//when coefficients of a certain column are all zeors, the inverse matrix does not exist
		{
		      printf("The inverse matrix does not exist!");
		}
		else if(k != j) 
		{
		     for (m = 0 ; m <= 5 ; m ++ )                  
		     {
			 swap(gaussian[k][m],gaussian[j][m]);
		     }
	
		}

		assert(gaussian[j][j] != 0); //to make sure that no starting point of each elimination process is zero

		for ( i = j + 1 ; i <= 2 ; i ++ )//eliminate each row of the lower triangular matrix
		{
		    if(gaussian[i][j] == 0)
			    continue;
		    else
		    {
			    double ratio;
			    ratio = gaussian[i][j]/gaussian[j][j];
			    for (m = j ; m <= 5 ; m ++ )
			    {
				gaussian[i][m] = gaussian[i][m] - gaussian[j][m]*ratio;
			    }
		    }
	
		}
    }

//perform back substitution from the last row of the upper triangular matrix and subtract x3,x2,x1 from each column   
    double temp;
    for (j = 2 ; j >= 0 ; j -- )
    {
        temp = gaussian[j][j];
        for (m = j ; m <= 5 ; m ++ )
        {
            gaussian[j][m] = gaussian[j][m]/temp;
        }
    
        for (i = 0 ; i < j ; i ++ )
        {
            double ratio;
            ratio = gaussian[i][j]/1;//calculate the ratio to multiply a row and subtract it from the default row
            for (m = j ; m <= 5 ; m ++ )
            {
                gaussian[i][m] = gaussian[i][m] - (ratio*gaussian[j][m]);
            }
	
        }
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

	return 0;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////


     
