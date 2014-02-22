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

#ifndef __MPI_H_INCLUDED__
#include <mpic.h>
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

#ifndef __FSTREAM_INCLUDED__
#define __FSTREAM_INCLUDED__
#include <fstream>
#endif

// version string
#define VERSION "count version 1.0"
// documentation string
#define DOC_STRING "Author: aykut\nCreated on November 23, 2012\n\nDescription:\nReports count of each distinct word in a document.\n\nUsage:\n./q1 -n 3 input_file\nPrints the most common three words in input_file.\nIn the event of a tie, the output is handled via an alphabetical sort.\n\nThe following options are supported:\n\t-n\tprint first k most common words, where k is argument to -n\n\t\toption\n\t-h\tprint help screen, if exists program ignores all\n\t\tinputs\n\t-v\tprint version, if exists program ignores other\n\t\toptions (except -h)\n\tIf no option is  specified, print all the words and their count.\n"

// return value constants
#define SUCCESS 0
#define FAILURE 1

// function prototypes
void parse_options(int argc, char ** argv, int& nflag, std::string& nvalue,
                   std::string& dvalue);
void print_result(std::vector<Count>& printQueue);
int get_value(std::string * nvalueString);

// function codes
void parse_options(int argc, char ** argv, int& nflag, std::string& nvalue,
                   std::string& dvalue) {
  // n flag, 1 if option n is present, 0 otherwise
  nflag = 0;
  int dflag = 0;
  // option will keep the option characters given
  char option;
  // getopt loop, getopt is used for parsing the UNIX options.
  // while loop will iterate over the given character options,
  // option variable will get the given character option value
  // at each iteration.
  while ((option = getopt (argc, argv, "hvn:d:")) != -1) {
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
        nflag = 1;
        nvalue.assign(optarg);
        break;
      case 'd':
        dflag = 1;
        dvalue.assign(optarg);
        break;
      // if an unknown option parsed, throw exception
      case '?':
        throw UnknownOptionException(optopt);
      default:
        throw AbnormalGetoptException();
    }
  }
  if (dflag==0) {
    throw DValueException();
  }
  if (dvalue[dvalue.size()-1] != '/') {
    dvalue += '/';
  }
}

/*
 * input: print queue, type std::vector<Count>
 * calls printResult methods of objects stored.
 */
void print_result(std::vector<Count>& printQueue) {
  for (std::vector<Count>::iterator it=printQueue.begin();
       it!=printQueue.end(); ++it) {
    it->printResult();
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
  std::string dvalue;
  // get options
  try {
    parse_options(argc, argv, nflag, nvalueString, dvalue);
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
  catch (DValueException e) {
    std::cerr << "-d option is compulsory." << std::endl; 
    return FAILURE;
  }
  if (nflag==0) {
    nvalue = 0;
  }
  else {
    nvalue = get_value(&nvalueString);
  }
  // end of parsing options


  // ==========================INITIALIZE MPI==================================
  // number of threads
  int numberOfThread;
  // rank of the thread running the code
  int myRank;
  MPI_Init (NULL, NULL);
  MPI_Comm_size (MPI_COMM_WORLD, &numberOfThread);
  MPI_Comm_rank (MPI_COMM_WORLD, &myRank);
  // ==========================END OF INITIALIZING MPI=========================

  // ==========================DOING MY PART OF THE WORK=======================
  double start;
  double end;
  MPI_Barrier(MPI_COMM_WORLD);
  start = MPI_Wtime();
  DIR * dirp;
  struct dirent *dp;
  int fileCounter=0;
  // each thread has its printing queue, it will print if its the right time,
  // otherwise continue processing other files and keep the objects in print
  // queue
  std::vector<Count> printQueue;

  if ((dirp = opendir(dvalue.c_str())) == NULL) {
    std::cerr << "couldn't open '.'" << std::endl;
    return 0;
  }
  do {
    if ((dp = readdir(dirp)) != NULL) {
      if (myRank==(fileCounter%numberOfThread)) {
        std::string filepath;
        filepath = dvalue;
        filepath += dp->d_name;
        Count c(filepath, nvalue);
        if(!c.solve()) {
          printQueue.push_back(c);
        }
      }
      else {
        dp = readdir(dirp);
      }
      fileCounter++;
    }
  } while (dp != NULL);
  // only the rank 0 changes/broadcasts the printRank value
  int printRank = -1;
  if (myRank == 0) {
    printRank = 0;
  }
  do {
    if (myRank==printRank) {
      print_result(printQueue);
    }
    if (myRank==0) {
      // compute who will print next
      printRank++;
    }
    MPI_Bcast(&printRank, 1, MPI_INT, 0, MPI_COMM_WORLD);
  } while (printRank!=numberOfThread);
  MPI_Barrier(MPI_COMM_WORLD);
  end = MPI_Wtime();
  MPI_Finalize();
  if (myRank==0) {
    std::cout << "Runtime is " << end-start << std::endl;
  }
  return SUCCESS;
  // ==========================END OF MY PART OF THE WORK======================
}