q1: count.cpp q1.cpp count.hpp
	g++ -Wall -g count.cpp q1.cpp -o q1

runq1: q1
	./q1 -n 4 /proj/spear1/cse411/*

q2: count.cpp q2.cpp count.hpp
	mpic++ -Wall -g count.cpp q2.cpp -o q2

runq2: q2
#	mpirun c2-7 -ssi rpi usysv ./q2 -d /proj/spear1/cse411 -n 4
	mpirun n2-7 -ssi rpi usysv ./q2 -d /tmp/ayb211 -n 4


q3: count.cpp q3.cpp count.hpp
	mpic++ -Wall -g count.cpp q3.cpp -o q3

runq3: q3
	mpirun n2-7 -ssi rpi usysv ./q3 -d /proj/spear1/cse411 -n 4

q4: count.cpp q4.cpp count.hpp
	mpic++ -Wall -g count.cpp q2.cpp -o q4

runq4: q4
	mpirun c2-7 -ssi rpi usysv ./q4 -d /proj/spear1/cse411 -n 4

clean:
	rm q1 q2 q3 q4
