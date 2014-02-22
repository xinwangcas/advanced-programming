#include <iostream>
#include <map>
#include <string.h>
#include <stdlib.h>

using namespace std;
/*
 * This is the program for Question 1 of the assignment #8 for CSE411
 * It implements the count of frequent words of mutilple files.
 *
 * author: Yi Luo (yil712)
 *
 */

//-------------------------------------------------------------------

/*
 * This function is used to judge if the input char is a punctuation.
 * Only LETTERS and DIGITS are not punctuations
 */
int isPunctuation(char c){
	if (((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9')))
		return 0;
	else
		return 1;
}

/*
 * This function is used to change the letter from Capital version 
 * into samll version.
 */
char toSmall(char c){
	if ((c >= 'A') && (c <= 'Z'))
		return c + 32;
	else
		return c;
}

int main(int argc, char** argv){
  int nTopWord_Flag = 0; // to indicate if the parameter contains -n
  int topN = 0; // to indicate the number of most frequent words
  char option;

  // To deal with parameters in the command line
  while ((option = getopt(argc, argv, "n:")) != -1){
  	switch (option){
		case 'n':
			nTopWord_Flag = 1;
			topN = atoi(optarg); 
			break;
		default:
			break;
	}	
  }
 
  // if the prameter contains -n
  if (nTopWord_Flag == 1){
	map<string, int> wordFrequency; // specify a map for all words in the document

	int fileNum; // number of input files
	FILE * fr = NULL;
  	for (fileNum = optind; fileNum < argc; fileNum++){
		fr = fopen(*(argv + fileNum), "r"); // read the desired file

		if (fr == NULL){// to handle file reading exceptions
			cout << "Cannot open the file - " << * (argv+fileNum) << endl;
			cout << endl;
			fclose(fr);
			continue;
		}

		char eachChar;
		string tempWord = ""; // potential words
		int tempWordFrequency = 0;
		while (fscanf(fr, "%c", &eachChar) != EOF){
			eachChar = toSmall(eachChar); // to transfer letters into the small version
		
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
	
		fclose(fr); // close the file

		// since we only need to find out the top N most frequent words,
		// so we do not need to sort the whole word list
		int maxFreq = 0;
		string maxWord;
  		map<string, int>::iterator iter;
 		int i; // this is used to iterate to top n frequent words

		if (topN > (int) wordFrequency.size()) // we only care about existing words
			topN = (int) wordFrequency.size(); 

		for (i = 0; i < topN; i++){
			for (iter = wordFrequency.begin(); iter != wordFrequency.end(); iter++) {
   				if ((*iter).second > maxFreq){
					maxFreq = (*iter).second;
					maxWord = (*iter).first;
				}
  			}
			cout << * (argv+fileNum) << ", "  << maxWord << ", " << maxFreq << endl;
			wordFrequency.erase(maxWord);
			maxFreq = 0;
  		}
		wordFrequency.clear(); // clear the word for a file
		cout << endl;
	}
  }

  return 0;
}
