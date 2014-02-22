This is the codes for assignment #8 of CSE411, which is created by Yi Luo (yil712).

./q1 is the codes for Question 1, you can directly "make" for compiling.
./q2 is the codes for Question 2, you can directly "make" for compiling.
./q3 is the codes for Question 3, you can directly "make" for compiling.
	The "copyFiles.cpp" is used to copy files from /proj/spear1/cse411/ to /tmp/yil712/
./q4 is the codes for Question 4, you can directly "make" for compiling.
	The "urls.txt" is the list of urls.

=======================================Question 2=============================================
In question 2, we will implement the MPI programming. Tasks are assigned to multiple nodes,
then their results are collected together for printing. Beacause around 11:59pm 12/12/2012, 
ten machines are down due to the heavy computation burden. Therefore, the following experiment
results are based on only ten hosts.

----experiment--> mpirun n0-x -ssi rpi usysv ./question2 -d /proj/spear/cse411/ -n 5

(x = 0, 1, 2, ..., 9)

nodeNum		time
n0-0		36.2996
n0-1		22.3779
n0-2		17.4303
n0-3		12.9581
n0-4		11.7711
n0-5		10.9524
n0-6		9.86689
n0-7		8.51125
n0-8		7.46390
n0-9		6.63000

----conclusions-->
a) "Whether you achieve a parallel speedup when running your code on an increasing number of 
nodes in the sunlab?"
Yes, I achieve a speedup when running my code on increasing nodes in the Sunlab.

=======================================Question 3=============================================
This question requires us to read files from /tmp/yil712/ rather than /proj/spear/cse411/. By 
doing this, files are read from local disk instead of the NFS. Beacause around 11:59pm 12/12/2012, 
ten machines are down due to the heavy computation burden. Therefore, the following experiment
results are based on only ten hosts.

----experiment--> mpirun n0-x -ssi rpi usysv ./question3 -d /tmp/yil712/ -n 5

(x = 0, 1, 2, ..., 9)

nodeNum		time
n0-0		35.3418
n0-1		22.1827
n0-2		17.2460
n0-3		13.0031
n0-4		11.3128
n0-5		10.8607
n0-6		9.76397
n0-7		8.42503
n0-8		7.42013
n0-9		6.89968

----conclusions-->
a) Above all, we can observe a very apparent reduction of running time from n0-0 to n0-9. This 
is achieved with the cooperation of increasing multiple nodes.

b) "Do you observe a speedup in this case? Why or why not?" (when comparing with Question 2)
Yes, I can observe a not dramatical speedup (around 0.1 in time). The reason for such speedup 
is that files are no longer read from a common file directory which is on another machine. 
Instead, nodes/processes read files from their local directory "/tmp/yil712/". Therefore, there
is no need to communiation/exchange files between the network for reading files. The speed of 
compuation thus accelarates.

=======================================Question 4=============================================
This question requires us to download files from the internet according to the given url list.
Beacause around 11:59pm 12/12/2012, ten machines are down due to the heavy computation burden.
Therefore, the following experiment results are based on only ten hosts. 

----experiment--> mpirun n0-x -ssi rpi usysv ./question4 -l urls.txt -n 5

(x = 0, 1, 2, ..., 9)

nodeNum		time
n0-0		91.9677
n0-1		47.6535
n0-2		32.4967
n0-3		12.8197
n0-4		11.2475
n0-5		18.7067
n0-6		17.5358
n0-7		18.0356
n0-8		21.1310
n0-9		21.3610

----conclusions-->
a) In the "ulrs.txt", the number of urls is also 92 which is the same as the number of files under 
/proj/spear1/cse411. Compare results with those in question 3, we can find that the loading of
files from the internet dramatically increases the time of computation. Based on this phenomenon, 
we can infer that the connection between the Sunlab and the outside internet impacts on the running
time.

b) "When you run with multiple nodes, do you oberserve a speedup now?" 
When running with multiple nodes may improve results, I observe a overall trend of speedup. However, 
from the above result we can see that the last few records have larger time consumption, I think this
is because the unstable network situations in the Sunlab during my testing time.

c) "What does this suggest about the presence or absence of bottlenecks in the network infrastructure 
that connects the sunlab to the outside world?" 
Another thing we can observe is that even though more nodes are involved for n0-9 than n0-7, the
running time of n0-9 is not reduced dramatically (even not reduced here). This is because when the 
number of nodes increases, the factor (bottlenect) really influces the running time is no long the 
number of nodes. Instead, it is the internet conncection/situation that really matters here.