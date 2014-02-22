/* 
 * File:   count.hpp
 * Author: aykut
 * Created on November 23, 2012
 *
 * This file has the class declerations and constants that are used by count.cpp
 *
 */

#ifndef __MAP_INCLUDED__
#define __MAP_INCLUDED__
#include <map>
#endif

#ifndef __EXCEPTION_INCLUDED__
#define __EXCEPTION_INCLUDED__
#include <exception>
#endif

#ifndef __STRING_INCLUDED__
#define __STRING_INCLUDED__
#include <string>
#endif

#ifndef __VECTOR_INCLUDED__
#define __VECTOR_INCLUDED__
#include <vector>
#endif

#ifndef __UTILITY__
#define __UTILITY__
#include <utility>
#endif

#ifndef __ITERATOR__
#define __ITERATOR__
#include <iterator>
#endif

class UnknownOptionException: public std::exception {
  char value;
public:
  UnknownOptionException(char valueInput) {value = valueInput;}
  ~UnknownOptionException() throw() {}
  char getValue() {return value;}
};

class AbnormalGetoptException: public std::exception {
public:
  AbnormalGetoptException() {}
};

class DValueException: public std::exception {
public:
  DValueException() {}
};

class LValueException: public std::exception {
public:
  LValueException() {}
};

class Exit: public std::exception {
public:
  Exit() {}
};

class Count {
  std::string filepath;
  int n;
  std::map<std::string, int> result;
  std::vector<std::pair<std::string, int> > sortedResult;
  void processWord(std::string&);
  void sort();
public:
  Count(std::string&, int);
  ~Count();
  int solve();
  void printResult();
  std::map<std::string, int> * getResult();
  std::string getFilepath();
  int getN();
};

struct compare {
  bool operator()(const std::pair<std::string, int> i,
                  const std::pair<std::string, int> j) {
    return (i.second >= j.second);
  }
};
