/* 
 * File:   q3.cpp
 * Author: aykut
 * Created on November 23, 2012
 *
 * Copy files in /proj/spear1/cse411 to /temp/ayb211/
 *
 */

// TODO
//
//

#include<stdio.h>
#include<mpi.h>
#include<string>

int main(int argc, char** argv) {
  // ==========================INITIALIZE MPI==================================
  // number of threads
  int numberOfThread;
  // rank of the thread running the code
  int myRank;
  MPI_Init (NULL, NULL);
  MPI_Comm_size (MPI_COMM_WORLD, &numberOfThread);
  MPI_Comm_rank (MPI_COMM_WORLD, &myRank);
  // ==========================END OF INITIALIZING MPI=========================

  std::string cmd1("mkdir /tmp/ayb211");
  std::string cmd2("cp /proj/spear1/cse411/* /tmp/ayb211/");
  popen(cmd1.c_str(), "r");
  popen(cmd2.c_str(), "r");
  MPI_Finalize();
  return 0;
}
