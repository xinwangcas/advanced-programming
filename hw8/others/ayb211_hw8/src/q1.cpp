/* 
 * File:   main.cpp
 * Author: aykut
 * Created on November 23, 2012
 *
 * Report count of each distinct word in a document using Count class.
 *
 * Usage:
 * ./main -n 3 input_file
 * prints the most common three words in input_file
 * In the event of a tie, the output is handled via an alphabetical sort.
 *
 *
 * The following options are supported:
 *    -n       print first k most common words, where k is argument to -n
               option
 *    -h       print help screen, if exists program ignores all
 *             inputs
 *    -v       print version, if exists program ignores other
 *             options (except -h)
 *     If no option is  specified, print all the words and their count.
 */

// TODO
//
//

#include "count.hpp"

#ifndef __IOSTREAM_INCLUDED__
#include <iostream>
#endif

#ifndef __UNISTD_H_INCLUDED__
#include <unistd.h>
#endif

#ifndef __VECTOR_INCLUDED__
#include <vector>
#endif

#ifndef __SSTREAM_INCLUDED__
#include <sstream>
#endif

#ifndef __DIRENT_H_INCLUDED__
#include <dirent.h>
#endif

// version string
#define VERSION "count version 1.0"
// documentation string
#define DOC_STRING "Author: aykut\nCreated on November 23, 2012\n\nDescription:\nReports count of each distinct word in a document.\n\nUsage:\n./q1 -n 3 input_file\nPrints the most common three words in input_file.\nIn the event of a tie, the output is handled via an alphabetical sort.\n\nThe following options are supported:\n\t-n\tprint first k most common words, where k is argument to -n\n\t\toption\n\t-h\tprint help screen, if exists program ignores all\n\t\tinputs\n\t-v\tprint version, if exists program ignores other\n\t\toptions (except -h)\n\tIf no option is  specified, print all the words and their count.\n"

// return value constants
#define SUCCESS 0
#define FAILURE 1

// function prototypes
void parse_options(int argc, char ** argv, int * nflag, std::string * nvalue,
                   std::vector<std::string> * fileArguments);

// function codes
void parse_options(int argc, char ** argv, int * nflag, std::string * nvalue,
                   std::vector<std::string> * fileArguments) {
  // n flag, 1 if option n is present, 0 otherwise
  *nflag = 0;
  // option will keep the option characters given
  char option;
  // getopt loop, getopt is used for parsing the UNIX options.
  // while loop will iterate over the given character options,
  // option variable will get the given character option value
  // at each iteration.
  while ((option = getopt (argc, argv, "hvn:")) != -1) {
    switch (option) {
      // if -h is present in the options print help and exit
      case 'h':
        std::cout << DOC_STRING << std::endl;
        throw Exit();
      // if -v is present in the options print version and exit
      case 'v':
        std::cout << VERSION << std::endl;
        throw Exit();
      // assign the related flags for existing options.
      case 'n':
        *nflag = 1;
        nvalue->assign(optarg);
        break;
      // if an unknown option parsed, throw exception
      case '?':
        throw UnknownOptionException(optopt);
      default:
        throw AbnormalGetoptException();
    }
  }
  for (int i=optind;i<argc;i++) {
    fileArguments->push_back(std::string(argv[i]));
  }
}

int get_value(std::string * nvalueString) {
  int value;
  std::stringstream ss(*nvalueString);
  ss >> value;
  return value;
}

int main(int argc, char** argv) {
  int nflag;
  std::string nvalueString;
  int nvalue;
  std::vector<std::string> fileArguments;
  // get options
  try {
    parse_options(argc, argv, &nflag, &nvalueString, &fileArguments);
  }
  catch (Exit e){
    return SUCCESS;
  }
  catch (UnknownOptionException e) {
    std::cerr << "Unkown option " << e.getValue() << std::endl;
    return FAILURE;
  }
  catch (AbnormalGetoptException e) {
    std::cerr << "Abnormal getopt behavior" << std::endl;
    return FAILURE;
  }
  if (nflag==0) {
    nvalue = 0;
  }
  else {
    nvalue = get_value(&nvalueString);
  }
  // end of parsing options

  // count the words for each file
  for(unsigned i=0;i<fileArguments.size();i++) {
    Count * c;
    c = new Count(fileArguments[i], nvalue);
    c->solve();
    c->printResult();
    c->getResult();
    delete c;
  }
  return SUCCESS;
}
