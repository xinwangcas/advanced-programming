//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//Matrix Parsing Code /////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

#include "StdAfx.h"


//Inside the result, result is stored as:
//i=0 i=1 i=2
//[0] [3] [6]  //j=0
//[1] [4] [7]  //j=1
//[2] [5] [8]  //j=2

double ** parseMatrixFile( char * inputFileName )
{
	///declare the file pointer, open the file.
	///read only.
	FILE * matrixFile = fopen(inputFileName, "r");
	
	//if the file does not exist, complain and exit.
	if(matrixFile == NULL){
		printf("ERROR: Matrix file [%s] not found.  Exitting.\n", inputFileName);
		exit(1);
	}
	else{
		//if the file does exist, notify the user that you are parsing it.
		printf("Parsing Matrix File: [%s]\n", inputFileName);
	}

	//declare the temporary variables that you need for parsing.
	char * line = new char[1000];			//stores the line from the file.
	char * lineCopy = new char[1000];		//copy of the line, used for user info.
	char * ptr;						//stores pointer to the next token
	double * tempMatrix = new double[9];	//stores the inputs pre-formatting
	int arrayPosition = 0;				//stores position in the array above.
	int i = 0;

	//parse the file to completion.
	while( fgets(line, 1000, matrixFile) != NULL){

		//tell the user what you are parsing.
		printf("PARSING: [%s]\n", line);
		
		//copy the line.
		strcpy(lineCopy, line);

		////check the line for degeneracy (i.e. empty)
		ptr = strtok(line, " \n\t\r");
		if(ptr == NULL){ continue; }
		
		////skip through the comments
		if(line[0] == '#'){ continue; }
		
		//now we assume that we have real information.  Parse the line.
		for(i = 0; i<3; i++){
			if( ptr == NULL ){
				printf("ERROR: you do not have enough numbers on this line [%s].  Exitting.", lineCopy);
				printf("       You need three numbers per line..");
				exit(1);
			}
			
			tempMatrix[ arrayPosition + (i*3) ] = atof(ptr);
			ptr = strtok(NULL, " \n\t\r");
		}
		arrayPosition++;

	}

	//inform the user that you are done.
	printf("Matrix File [%s] Parsing Complete. \n", inputFileName);

	//clean up from parsing.
	delete[](line);
	delete[](lineCopy);
	
	//Think of the matrix arrangement inside arrayPosition as follows:
	//the arrays are stored as follows:
	//[0, 3. 6] //first line in file
	//[1, 4. 7] //second line in file
	//[2, 5. 8] //third line in file
	
	//now convert the matrix into the two dimensional matrix.
	double ** result = new double*[3];
	int j = 0;
	for(i = 0; i<3; i++){
		result[i] = new double[3];
		for(j = 0; j<3; j++){
			result[i][j] = tempMatrix[ 3*i+j ];
		}
	}

	//Inside the result, result is stored as:
	//i=0 i=1 i=2
	//[0] [3] [6]  //j=0
	//[1] [4] [7]  //j=1
	//[2] [5] [8]  //j=2
	
	//clean up
	delete[](tempMatrix);

	//return results
	return result;
}




////prints out a matrix
void printMatrix(double ** mat)
{
	printf("%06f %06f %06f \n", mat[0][0], mat[1][0], mat[2][0]);
	printf("%06f %06f %06f \n", mat[0][1], mat[1][1], mat[2][1]);
	printf("%06f %06f %06f \n", mat[0][2], mat[1][2], mat[2][2]);
	
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//Matrix Parsing Code /////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////




