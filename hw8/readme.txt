/*****************************************************************
*Xin Wang
*xiw412
*****************************************************************/

******************************************************************

usage method:

******************************************************************

Problem1:
#Here I use std::map to count each distinct word in a document
#my program ignore all punctuation and ignore capitalization
#my program can be configurable by changing the parameter of n
#my program can deal with more than one input file.
#my program uses getopt function to handle command-line parameters

Enter directory problem1/
g++ q1.cpp -o q1
./q1 -n 3 swim.txt

or you can just 
make
./q1 -n 3 swim.txt

******************************************************************

Problem2:
#Here I extend my program to use MPI.
#Now my program takes -d and -n parameters.

Enter directory problem2/
recon hostfile.txt
lamboot -v hostfile.txt
mpic++ problem2.cpp -o problem2 (or just make)
mpirun n0-19 -ssi rpi usysv ./problem2 -n3 -d /proj/spear1/cse411/
result:The running time is: 9.252367
mpirun n0-1 -ssi rpi usysv ./problem2 -n3 -d /proj/spear1/cse411/
result:The running time is: 25.65335
I will explain this further in my README.pdf

******************************************************************

Problem3:
#I provide an extra file to spread files to /tmp/xiw412 to each machine

cd problem3/
mpic++ q3.cpp -o q3(or just make)
mpirun n0-19 -ssi rpi usysv ./q3 -d /proj/spear1/cse411/
mpirun n0-19 -ssi rpi usysv ./problem3 -n3 -d /tmp/xiw412/
(or you can just "bash run.sh")
result:The running time is: 9.084685
There is a speed up in this case.
I will expain this further in my README.pdf

******************************************************************

Problem4:
#Here the program takes a -l parameter instead of a -d parameter
#THe URLs are listed in web.txt
#the system call wget is used to parse URLs

cd problem4/
mpic++ q4.cpp -o q4 (or just make)
mpirun n0-19 -ssi rpi usysv ./q4 -n3 -l ./web.txt 
(or just "bash run.sh")
result:The running time is: 49.693000
mpirun n0-1 -ssi rpi usysv ./q4 -n3 -l ./web.txt 
result:The running time is: 2.279772
I will explain this further in my README.pdf.

******************************************************************
Note: for detailed comments, please read /problem2/problem2.cpp
