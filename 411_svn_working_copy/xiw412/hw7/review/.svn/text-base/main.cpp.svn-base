//---------------------------------------------------------------------------------------------------------------/
// EDITED BY: William West
// ORIGINAL AUTHOR: Prof. Brian Chen
// CSC 411 - Programming Methods
// Homework 6
// November 5, 2012
// Filename: main.cpp
// Program Description: Given a 3x3 matrix, computes the inverse matrix. Computation is done both with and without
//					    partial pivoting.
// To Use: compile use: "%make clean" THEN "%make all" (If using linux, use gmake instead of make)
//						Then, navigate to /debug, and execute using %invert matrix.txt
//---------------------------------------------------------------------------------------------------------------/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Implementation for the Main method
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"


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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//DESCRIPTION: Computes the simple inverse matrix of a 3x3 matrix. No partial pivoting is used. All precision is shown to 6
//decimal points in output, but is computed with the default precision of the double value.
//NOTE: Computation is done using the standard [row][column] notation, and transposed before using printMatrix function.
/***************************************************************************************************************************/
//INPUT: 3x3 Matrix of double values
//OUTPUT: 3x3 Inverse matrix of input
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double ** simpleMatrixInversion( double ** matrix ){
	int i = 0;
	int j = 0;
	int k = 0;
	double ** tempMatrix = new double*[3]; // 3x6 matrix, stores original matrix and identity matrix. used for calculations.
	
	//initialize temp matrix to 0 values
	for(i=0; i<3; i++){
		tempMatrix[i] = new double[6];
		for(j=0; j<6; j++){
			tempMatrix[i][j]=0.0;
		}
	}
	
	//Create identity matrix
	tempMatrix[0][3] = 1.0;
	tempMatrix[1][4] = 1.0;
	tempMatrix[2][5] = 1.0;
	
	//initialize temp matrix to orig matrix values
	for(i=0; i<3; i++){
		for(j=0; j<3;j++){
			tempMatrix[i][j]=matrix[j][i];
		}
	}
	
	/////////////////////////////////////////////////
	////////////COMPUTE TRIANGULAR MATRIX////////////
	/////////////////////////////////////////////////
	for(k=0; k<3; k++){
		for(i=k+1; i<3; i++){ //for each row starting at row 2
			double elimCoefficient = tempMatrix[i][k]/tempMatrix[k][k];
			for(j=k; j<6; j++){
				tempMatrix[i][j] = tempMatrix[i][j]-(elimCoefficient*tempMatrix[k][j]);
			}
		}
	}
	
	////////////////////////////////////
	////////////BACKTRACKING////////////
	////////////////////////////////////
	for(k=2; k>-1; k--){		//for each column, starting at column 3
		for(j=3; j<6; j++){
			tempMatrix[k][j] = tempMatrix[k][j]/tempMatrix[k][k];
		}
		tempMatrix[k][k] = tempMatrix[k][k]/tempMatrix[k][k];
		for(i=k-1; i>-1; i--){		//for each row, starting at row 2
			for(j=3; j<6; j++){		//for each column of the inverse matrix
				tempMatrix[i][j] = tempMatrix[i][j] - (tempMatrix[i][k]*tempMatrix[k][j]);	//substitute ak into b values
			}
			tempMatrix[i][k] = tempMatrix[i][k]-tempMatrix[i][k]; //eliminate ak from identity matrix after substituting
		}
	}
	
	//declare and allocate the result
	double ** result = new double*[3];
	for(i = 0; i<3; i++){
		result[i] = new double[3];
		for(j=0; j<3; j++){
			result[i][j]= tempMatrix[j][i+3];
		}
	}
	return result;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//DESCRIPTION: Computes the inverse matrix of a 3x3 matrix. Partial pivoting is used at each column pivot. 
//NOTE: Computation is done using the standard [row][column] notation, and transposed before using printMatrix function.
/***************************************************************************************************************************/
//INPUT: 3x3 Matrix of double values
//OUTPUT: 3x3 Inverse matrix of input
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double ** partialPivotInversion( double ** matrix ){
	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;
	double temp = 0.0;
	double ** tempMatrix = new double*[3];
	
	//initialize temp matrix to 0 values
	for(i=0; i<3; i++){
		tempMatrix[i] = new double[6];
		for(j=0; j<6; j++){
			tempMatrix[i][j]=0.0;
		}
	}
	
	//Create identity matrix
	tempMatrix[0][3] = 1.0;
	tempMatrix[1][4] = 1.0;
	tempMatrix[2][5] = 1.0;
	
	//initialize temp matrix to orig matrix values
	for(i=0; i<3; i++){
		for(j=0; j<3;j++){
			tempMatrix[i][j]=matrix[j][i];
		}
	}
	
	/////////////////////////////////////////////////
	////////////COMPUTE TRIANGULAR MATRIX////////////
	/////////////////////////////////////////////////
	for(k=0; k<2; k++){
		//partial pivoting
		for(l=k+1; l<3; l++){
			if(fabs(tempMatrix[k][k])<fabs(tempMatrix[l][k])){
				for(j=0; j<6; j++){
					temp = tempMatrix[k][j];
					tempMatrix[k][j]=tempMatrix[l][j];
					tempMatrix[l][j]=temp;
				}
			}
		}
		for(i=k+1; i<3; i++){ 	//for each row starting at row 2
			double elimCoefficient = tempMatrix[i][k]/tempMatrix[k][k];
			for(j=k; j<6; j++){
				tempMatrix[i][j] = tempMatrix[i][j]-(elimCoefficient*tempMatrix[k][j]);
			}
		}
	}
	
	////////////////////////////////////
	////////////BACKTRACKING////////////
	////////////////////////////////////
	//for each column, starting at column 3
	for(k=2; k>-1; k--){
		for(j=3; j<6; j++){
			tempMatrix[k][j] = tempMatrix[k][j]/tempMatrix[k][k];
		}
		tempMatrix[k][k] = tempMatrix[k][k]/tempMatrix[k][k];
		for(i=k-1; i>-1; i--){		//for each row, starting at row 2
			for(j=3; j<6; j++){		//For each column of the inverse matrix
				tempMatrix[i][j] = tempMatrix[i][j] - (tempMatrix[i][k]*tempMatrix[k][j]);	//substitute ak into b values
			}
			tempMatrix[i][k] = tempMatrix[i][k]-tempMatrix[i][k];	//eliminate ak from identity matrix after substituting
		}
	}

	//declare and allocate the result
	double ** result = new double*[3];
	for(i = 0; i<3; i++){
		result[i] = new double[3];
		for(j=0; j<3; j++){
			result[i][j]= tempMatrix[j][i+3];
		}
	}
	return result;
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

// ###     ###     ###    ####### ###     ###    ###     ### ####### ####### ###   ###   ######   ######  
// ####   ####    #####   ####### ####    ###    ####   #### ####### ####### ###   ###  ########  ####### 
// ##### #####    #####     ###   #####   ###    ##### ##### ###       ###   ###   ### ########## ########
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
	if( argc == 2 && ( strcmp(argv[1], "-format")==0 ) ){ 	//if 1 argument is provided and the first argument is -format
		fileFormat();
	}

	///invert
	if( argc == 2 && ( strcmp(argv[1], "-format")!=0 ) ){
		double ** matrix = parseMatrixFile( argv[1] );
		printMatrix(matrix);
		printf("\n");
		
		//compute the simple inverse and print it
		printf("COMPUTING SIMPLE INVERSE MATRIX\n");
		double ** simpleInverse = simpleMatrixInversion( matrix );
		printMatrix(simpleInverse);
		printf("\n");
		
		//compute the inverse with partial pivoting and print it
		printf("COMPUTING PARTIAL INVERSE MATRIX\n");
		double ** partialInverse = partialPivotInversion( matrix );
		printMatrix(partialInverse);
		printf("\n");
		
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


