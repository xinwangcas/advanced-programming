#include<stdio.h>
#include<cstdlib>
#include<iostream>
#include<string.h>
#include<fstream>
#include<dirent.h>
#include<mpicxx.h>
#include<sys/types.h>
#include<sys/stat.h>

using namespace std;

/*
 * This program copies files from "/proj/spear1/cse411/" to "/tmp/yil712",
 * which is used for Question 3.
 */

int main(int argc, char ** argv){
	int rank, size;
	MPI::Status status;
	MPI::Init(argc, argv);
	rank = MPI::COMM_WORLD.Get_rank(); // to get the current rank
	size = MPI::COMM_WORLD.Get_size(); // to get the number of nodes
   	
	string from = "/proj/spear1/cse411/"; // the from address
	string to = "/tmp/yil712/"; // the to address

	//cout<< rank << ", " << system("mkdir /tmp/yil712/") << endl;
	
	/**
	 * To create the directory of "to", similar to "mkdir"
	 */
	int mkdirFlag;
	if ((mkdirFlag = mkdir(to.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) != 0) // mkdir
		cout << to << " already exists. We don't need to re-create it on rank = " << rank << endl;
	else 
		cout << to << " has been successfully created on rank = " << rank << endl;

	
       	DIR *pDIR;
	struct dirent *entry;
   	if( (pDIR=opendir(from.c_str())) != NULL ){
	       	while((entry = readdir(pDIR)) != NULL){ // to iterate all files in the "from" directory
			if( strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 ){
				FILE * fromFile, * toFile;
				fromFile = fopen((from + string(entry->d_name)).c_str(), "r");
				toFile = fopen((to + string(entry->d_name)).c_str(), "w");
				if (fromFile == NULL)
					cout << "No such file \'" << entry->d_name << "\' in " << from << endl;
				if (toFile == NULL)
					cout << "No such file \'" << entry->d_name << "\' in " << to << endl;
				
				// to copy files directly
				char tempChar;
				while (fscanf(fromFile, "%c", &tempChar) != EOF)
					fprintf(toFile, "%c", tempChar);
				
				fclose(fromFile); // to close file
				fclose(toFile); // to close file

				cout << "rank = " << rank << " of " << size << ", copies \'";
				cout << entry->d_name << "\' from " << from << " to " << to << endl;			   }		
        	}
     	 	closedir(pDIR);
    	}
	return 0;	
}
