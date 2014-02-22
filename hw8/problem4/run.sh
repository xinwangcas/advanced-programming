#!/bin/bash

rm -fr output.txt

for nodes in `seq 9`
do
	echo "running the program with #nodes = " `expr $nodes + 1` 
	echo "running the program with #nodes = " `expr $nodes + 1`  >> output.txt 
	mpirun n0-$nodes -ssi rpi usysv ./q4 -n3 -l ./web.txt >> output.txt
done
