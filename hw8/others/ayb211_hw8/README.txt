./src/count.hpp: header file for Count class and various exception classes
./src/count.cpp: source code for Count class method definitions

./src/q1.cpp: source code for solving question 1, uses Count class to do the
work per file input
./src/q2.cpp: source code for solving question 2, uses Count class to do the
work per file input
./src/q3.cpp: source code for solving question 3, uses Count class to do the
work per file input
./src/q4.cpp: source code for solving question 4, uses Count class to do the
work per file input

./src/Makefile   provides the following
make q1:    compiles q1 (with g++)
make q2:    compiles q2 (with mpic++)
make q3:    compiles q3 (with mpic++)
make q4:    compiles q4 (with mpic++)
make runq1: runs q1 with following options: ./q1 -n 4 /proj/spear1/cse411/*
make runq2: runs q2 with following options: mpirun c2-7 -ssi rpi usysv ./q2
-d /proj/spear1/cse411 -n 4
make runq3: runs q3 with following options: mpirun c2-7 -ssi rpi usysv ./q3
-d /proj/spear1/cse411 -n 4
make runq4: runs q4 with following options: mpirun c2-7 -ssi rpi usysv ./q4
-d /proj/spear1/cse411 -n 4
You can change ./src/Makefile for running with the desired parameters.

./src/setup_for_mpi.sh: A script to set the environment for mpi
./src/done.sh         : You should call this when you are done (to undo the
changes of setup_for_mpi)