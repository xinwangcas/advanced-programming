#!/bin/bash

mpirun n0-9 -ssi rpi usysv ./q3 -d /proj/spear1/cse411/

rm -fr output.txt

for nodes in `seq 9`
do
	echo "running the program with #nodes = " `expr $nodes + 1` 
	echo "running the program with #nodes = " `expr $nodes + 1`  >> output.txt 
	mpirun n0-$nodes -ssi rpi usysv ./problem3 -n3 -d /tmp/xiw412/  >> output.txt
done
