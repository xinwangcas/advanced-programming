#include <iostream>
#include <map>
#include <stdlib.h>
#include <string.h>
#include <cstdlib>
#include <fstream>
#include <dirent.h>
#include <vector>
#include <mpicxx.h>

using namespace std;
/*
 * For Question 4 of HW #8 of CSE411.
 * This program achieves the MPI programming. The lead process (rank =0) get the list 
 * of urls and distribute urls to multiple nodes. Nodes then download files from the
 * Internet and then calculate frequent words as required.
 * A timer is set to record the running time. We can observe the change of running 
 * time when the number of nodes changes.
 *
 * author: Yi Luo (yil712)
 *
 */

//--------------------------------------------------------------------------------------

/*
 * To judge if the char is a punctuation.
 * Here we assume that any char that is not a LETTER or a DIGIT should be a punctuation.
 */
int isPunctuation(char c){
	if (((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9')))
		return 0;
	else
		return 1;
}

/*
 * To change capital chars into small chars.
 */
char toSmall(char c){
	if ((c >= 'A') && (c <= 'Z'))
		return c + 32;
	else
		return c;
}

/*
 * To get the top n most frequent words from a given file.
 * This method records the implementing rank, the number of total tasks for this rank,
 * and the number of processing tasks (the number of current task). 
 */
string getTopNWordFrom(char* fileName, int topN, int rank, int total, int number){
	string result = "";// to save the result
	
	map<string, int> wordFrequency;
        FILE * fr = NULL;
        fr = fopen(fileName, "r");

		if (fr == NULL){// to handle with unexpected file input
			cout << "Cannot open the file - " << fileName  << endl;
			cout << endl;
			fclose(fr);
			return result;
		//	return 0;
		}

		char eachChar;
		string tempWord = "";
		int tempWordFrequency = 0;// potential words
		while (fscanf(fr, "%c", &eachChar) != EOF){
			eachChar = toSmall(eachChar);// change capital letters into small versions
		
			if (isPunctuation(eachChar) == 0){
				tempWord += eachChar;
			} else if (tempWord != ""){
				if (wordFrequency.find(tempWord) != wordFrequency.end()){
					tempWordFrequency = wordFrequency.find(tempWord)->second;
					tempWordFrequency ++;
					//cout << "find: " << tempWordFrequency  << endl;
				}else
					tempWordFrequency = 1;
				wordFrequency[tempWord] =  tempWordFrequency;
				//cout << wordFrequency[tempWord] << endl;
				tempWord = "";
			}
		}
	
		fclose(fr);// to close the file

		int maxFreq = 0;
		string maxWord;
  		map<string, int>::iterator iter;
 		int i;// to find out the top n most frequent words
		for (i = 0; i < topN; i++){
			for (iter = wordFrequency.begin(); iter != wordFrequency.end(); iter++) {
   				if ((*iter).second > maxFreq){
					maxFreq = (*iter).second;
					maxWord = (*iter).first;
				}
  			}

			char buff[1024];
			sprintf(buff, "rank = %d, total_Task = %d, number_Task = %d, ", rank, total, number);
			result += string(buff);
			result += string(fileName) + ", " + string(maxWord) + ", ";
			sprintf(buff, "%d", maxFreq);
			result += string(buff)+"\n";

		//	cout << "rank = " << rank << ", total = " << total << ", number = " << number;
		//	cout << ", " << fileName << ", " << maxWord << ", " << maxFreq <<  endl;
			wordFrequency.erase(maxWord);
			maxFreq = 0;
  		}
	result += "\n";    
//	cout << endl;
	wordFrequency.clear();
	return result;
//	return 1;
}

int main(int argc, char** argv){
  int nTopWord_Flag = 0; // the flag of parameter n
  int topN = 1; // to calculate top n words, default is 1
  int direct_Flag = 0; // to indicate if reading files from a directory
  char * directName; // to indicate the name of directory of files, e.g. /proj/spear1/cse411
  int url_Flag = 0; // to indicate if the input is url list
  char * urlFileName; // to specify the file name for the url list
  char option;

  char your_user_name[100] = "yil712"; // the user name 

  int rank, size;
  MPI::Status status;
  MPI::Init(argc, argv);
  rank = MPI::COMM_WORLD.Get_rank(); // to get current rank number 
  size = MPI::COMM_WORLD.Get_size(); // to get the number of nodes

  double t1, t2;// for timing

  while ((option = getopt(argc, argv, "d:n:l:")) != -1){
  	switch (option){
		case 'n':// to indicate top n frequent words
			nTopWord_Flag = 1;
			topN = atoi(optarg);
			break;
		case 'd':// to indicate the directory of files
            		direct_Flag = 1;
			directName = optarg;
            		break;
		case 'l':// to indicate the url file
			url_Flag = 1;
			urlFileName = optarg;
		default:
			break;
	}	
  }
 
  if (nTopWord_Flag == 1){//if the parameter contains -n, deal with a certain file on all nodes 

	int fileNum;
	string result;
  	for (fileNum = optind; fileNum < argc; fileNum++){
		result = getTopNWordFrom(* (argv + fileNum), topN, 0, argc - optind, fileNum - optind + 1); 
		//if (getTopNWordFrom(* (argv + fileNum), topN, 0, argc - optind, fileNum - optind + 1)  == 0)
		if (result == "")
			continue;
		else 
			cout << result;
	}
  }//endif option -n
  

  if (direct_Flag == 1){	// for -d
	if (rank == 0){
  		t1 = MPI_Wtime();
		vector<string> fileList; // to record the list of files
		DIR *pDIR;
		struct dirent *entry;
  		if( (pDIR = opendir(directName)) != NULL ){// iterate within a directory
        		while((entry = readdir(pDIR)) != NULL){// iterate for all files
	             		if( strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 ){									fileList.push_back(string(directName) + string(entry->d_name));
				}
			}
			closedir(pDIR);// close the DIR
		}
		vector<string>::iterator iter;

		iter = fileList.begin();

		int fileNum = fileList.size();
		int chunkSize = fileNum / size;  // to get the basic size of a chunk for distribution
		int restNode = fileNum % size; // to get the remaining tasks for further distribution

		int chunk_size[1]; // to convey the chunk size for each node

		string data; // to convey the file name
		int count; // a parameter for sending
								
		int moreTaskNode; // nodes who deal with ONE more tasks than others

		for (moreTaskNode = 1; moreTaskNode <= restNode; moreTaskNode++){
			chunk_size[0] = chunkSize + 1; // one more task
			MPI::COMM_WORLD.Send(chunk_size, 1, MPI::INT, moreTaskNode, 411); // send function
			for (int i = 0; i < chunkSize+1; i++){
				data = * iter; // to iterate the file
				count = data.length()+1;
				MPI::COMM_WORLD.Send(data.c_str(), count, MPI::CHAR, moreTaskNode, 411); // send function
				iter ++;
				}
		}

		int lessTaskNode; // nodes who deal with less tasks
		for (lessTaskNode = moreTaskNode; lessTaskNode < size; lessTaskNode++){
			chunk_size[0] = chunkSize;
			MPI::COMM_WORLD.Send(chunk_size, 1, MPI::INT, lessTaskNode, 411); // send function
			for (int i = 0; i < chunkSize; i++){
				data = * iter; // to iterate the file
				count = data.length()+1;
				MPI::COMM_WORLD.Send(data.c_str(), count, MPI::CHAR, lessTaskNode, 411); // send function
				iter ++;
				}
		}
		
		// Then the rank = 0 node will deal with the rest tasks by itself
		int number = 0;
		for (int i = 0; i < chunkSize; i++){
			number ++;
			cout << getTopNWordFrom((char *)(* iter).c_str(), topN, rank, chunkSize, number);
			iter ++;
		}

		// at last, the rank = 0 node collects results from all nodes and get the running time
		char buffResult[1024];
		for (int i = 0; i < fileNum - chunkSize; i++){
			MPI::COMM_WORLD.Recv(buffResult, 1024, MPI::CHAR, MPI::ANY_SOURCE, MPI::ANY_TAG, status); // recv function
			cout << buffResult;
		}

  		t2 = MPI_Wtime();
		cout << "Elapsed time is: " << t2 - t1 << endl;
	}else{// if rank != 0
		int chunk_size[1];
		int number = 0;
		// to get the chunk size of this rank
		MPI::COMM_WORLD.Recv(chunk_size, 1, MPI::INT, MPI::ANY_SOURCE, MPI::ANY_TAG, status); // recv function
			
		string eachResult;
		int eachResult_count;
		for (int i = 0; i < chunk_size[0]; i++){
			char buffFile[1024];
			MPI::COMM_WORLD.Recv(buffFile, 1024, MPI::CHAR, MPI::ANY_SOURCE, MPI::ANY_TAG, status); // recv function
			number ++;
			eachResult = getTopNWordFrom(buffFile, topN, rank, chunk_size[0], number);
			eachResult_count = eachResult.length() + 1;
			MPI::COMM_WORLD.Send(eachResult.c_str(), eachResult_count, MPI::CHAR, 0, 318); // send back the result
		}
	}
  }//endif opiton is -d


  if (url_Flag == 1){ // for -l
	  if (rank == 0){
  		t1 = MPI_Wtime();
		vector<string> urlList;
		
		FILE * urlFile;
		urlFile = fopen(urlFileName, "r"); // to open the file of the url list

		if (urlFile == NULL){
			cout << "No such file " << urlFileName << endl;
		}

		char url[1024];

       		while(fscanf(urlFile, "%s", url) != EOF){ // push all url into a vector
             	//	cout << url << endl;
			urlList.push_back(url);
		}
		
		fclose(urlFile); // to close the file
		

		vector<string>::iterator iter;
		iter = urlList.begin();

		int urlNum = urlList.size();
		int chunkSize = urlNum / size;  // to get the basic size of a chunk for distribution
		int restNode = urlNum % size; // to get the remaining tasks for further distribution

		int chunk_size[1]; // to convey the chunk size for each node

		string data; // to convey the file name
		int count; // a parameter for sending
								
		int moreTaskNode; // nodes who deal with ONE more tasks than others

		for (moreTaskNode = 1; moreTaskNode <= restNode; moreTaskNode++){
			chunk_size[0] = chunkSize + 1; // one more task
			MPI::COMM_WORLD.Send(chunk_size, 1, MPI::INT, moreTaskNode, 411); // send function
			for (int i = 0; i < chunkSize+1; i++){
				data = * iter; // to iterate the file
				count = data.length()+1;
				MPI::COMM_WORLD.Send(data.c_str(), count, MPI::CHAR, moreTaskNode, 411); // send function
				iter ++;
				}
		}

		int lessTaskNode; // nodes who deal with less tasks
		for (lessTaskNode = moreTaskNode; lessTaskNode < size; lessTaskNode++){
			chunk_size[0] = chunkSize;
			MPI::COMM_WORLD.Send(chunk_size, 1, MPI::INT, lessTaskNode, 411); // send function
			for (int i = 0; i < chunkSize; i++){
				data = * iter; // to iterate the file
				count = data.length()+1;
				MPI::COMM_WORLD.Send(data.c_str(), count, MPI::CHAR, lessTaskNode, 411); // send function
				iter ++;
				}
		}

		// Then the rank = 0 node will deal with the rest tasks by itself
		int number = 0;
		for (int i = 0; i < chunkSize; i++){
			number ++;

			char localName[1024];
			sprintf(localName, "/tmp/%s/%d.txt", your_user_name, number); // tp specify the directory for saving downloaded files

			char buffComm[1024];
			sprintf(buffComm, "wget %s -O %s -q", (* iter).c_str(), localName); // download files - as a common line 
			//system(buffComm);

			cout << "rank = " << rank << ", commStatus = " << system(buffComm) << ", " << buffComm << endl;

			cout << getTopNWordFrom(localName, topN, rank, chunkSize, number);

			sprintf(buffComm, "rm -rf %s", localName);  // delete files - as a common line 
			system(buffComm);
			iter ++;
		}
		
		// at last, the rank = 0 node collects results from all nodes and get the running time
		char buffResult[1024];
		for (int i = 0; i < urlNum - chunkSize; i++){
			MPI::COMM_WORLD.Recv(buffResult, 1024, MPI::CHAR, MPI::ANY_SOURCE, MPI::ANY_TAG, status);
			cout << buffResult;
		}

  		t2 = MPI_Wtime();
		cout << "Elapsed time is: " << t2 - t1 << endl;
	}else{// if rank != 0
		int chunk_size[1];
		int number = 0;
		// to get the chunk size of this rank
		MPI::COMM_WORLD.Recv(chunk_size, 1, MPI::INT, MPI::ANY_SOURCE, MPI::ANY_TAG, status); // recv function
			
			
		string eachResult;
		int eachResult_count;
		for (int i = 0; i < chunk_size[0]; i++){
			char buffFile[1024];
			MPI::COMM_WORLD.Recv(buffFile, 1024, MPI::CHAR, MPI::ANY_SOURCE, MPI::ANY_TAG, status);
			number ++;
			
			char localName[1024];
			sprintf(localName, "/tmp/%s/%d.txt", your_user_name, number);// tp specify the directory for saving downloaded files
			
			char buffComm[1024];
			sprintf(buffComm, "wget %s -O %s -q", buffFile, localName); // download files - as a common line 
			//system(buffComm);
		
			cout << "rank = " << rank << ", commStatus = " << system(buffComm) << ", " << buffComm << endl;

			eachResult = getTopNWordFrom(localName, topN, rank, chunk_size[0], number);
			eachResult_count = eachResult.length() + 1;
			MPI::COMM_WORLD.Send(eachResult.c_str(), eachResult_count, MPI::CHAR, 0, 318);
			
			sprintf(buffComm, "rm -rf %s", localName);  // delete files - as a common line 
			system(buffComm);
		}
	}
  }//endif option is -l

  return 0;
}
