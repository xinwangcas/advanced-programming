/* 
 * File:   count.cpp
 * Author: aykut
 * Created on November 23, 2012
 *
 * This file has the Count class definitions
 *
 * If there exists a punctuation in between characters, then characters are
 * splitted and treated as two words.
 *
 * Ex: red.apple is treated as two different words, red and apple.
 *
 */

// TODO
//
//

#include "count.hpp"

#ifndef __FSTREAM_INCLUDED__
#define __FSTREAM_INCLUDED__
#include <fstream>
#endif

#ifndef __IOSTREAM_INCLUDED__
#define __IOSTREAM_INCLUDED__
#include <iostream>
#endif

#ifndef __IOMANIP_INCLUDED__
#define __IOMANIP_INCLUDED__
#include <iomanip>
#endif

#ifndef __CCTYPE_INCLUDED__
#define __CCTYPE_INCLUDED__
#include <cctype>
#endif

#ifndef __ALGORITHM_INCLUDED__
#define __ALGORITHM_INCLUDED__
#include <algorithm>
#endif

#ifndef __STAT_H_INCLUDED__
#define __STAT_H_INCLUDED__
#include <sys/stat.h>
#endif


Count::Count (std::string& filepathInput, int nvalue) {
  filepath = filepathInput;
  n = nvalue;
}

Count::~Count() {
}

int Count::getN () {
  return n;
}

std::string Count::getFilepath() {
  return filepath;
}

// returns non-zero when solve fails.
int Count::solve() {
  std::ifstream fileStream;
  std::string word;
  try {
    fileStream.open(filepath.c_str());
  }
  // catch fstream failure exception and return 1
  catch (std::fstream::failure e) {
    std::cerr << "Error in openning file " << filepath
              << ", counting failed." << std::endl;
    return 1;
  }
  struct stat fileStat;
  if(stat(filepath.c_str(),&fileStat) < 0) {
    // file reading error
    std::cerr << "Error in openning file " << filepath
              << ", counting failed." << std::endl;
    return 2;
  }
  if (S_ISDIR(fileStat.st_mode)) {
    // fstream open is successful but input is a directory not a file
    std::cerr << "Input is a directory, not a file, " << filepath
              << ", counting failed." << std::endl;
    return 3;
  }
  while (fileStream >> word) {
    processWord(word);
  }
  fileStream.close();
  return 0;
}

void Count::sort() {
  std::map<std::string, int>::iterator it;
  std::pair<std::string, int> p;
  for (it=result.begin();it!=result.end();++it) {
    p.first = it->first;
    p.second = it->second;
    sortedResult.push_back(p);
  }
  // what if n is less then the number of words in result, i.e. size of sortedResult?
  if ((unsigned int) n>sortedResult.size()) {
    n = sortedResult.size();
  }
  std::partial_sort(sortedResult.begin(), sortedResult.begin()+n, sortedResult.end(), compare());
}

void Count::processWord(std::string& word) {
  std::string temp;
  temp = word;
  int flag;
  for (unsigned int i=0;i<word.length();i++) {
    flag = 0;
    if(!isalnum(word[i])) {
      flag = 1;
      if (i==0) {
        word = word.erase(0, 1);
        processWord(word);
        break;
      }
      else if (i==word.length()-1) {
        word = word.erase(i, std::string::npos);
        result[word]++;
        //no need for break since this is the end of loop
        //break;
      }
      // punctiation is in the middle of the word
      else {
        temp = temp.erase(0, i+1);
        word = word.erase(i, std::string::npos);
        result[word]++;
        processWord(temp);
        break;
      }
    }
    else if (isupper(word[i]))
      word[i] = tolower(word[i]);
  }
  if (flag==0 && word.length())
    result[word]++;
}

std::map<std::string, int> * Count::getResult() {
  return &result;
}

void Count::printResult() {
  sort();
  std::map<std::string, int>::iterator it;
  unsigned int size = 50;
  if (filepath.size()>size)
    size = filepath.size()+1;
  for(int i=0;i<n;i++) {
    std::cout << std::setw(size) << filepath << std::setw(14) << sortedResult[i].first << std::setw(10) << sortedResult[i].second << std::endl;
  }
}
