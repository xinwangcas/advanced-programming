/*
* Author: Xin Wang
* Project Name: wc
* Function: Count number of characters, words, bytes and lines in standard input or files
* Finish Date: Sep 16th, 2012
*/

INSTRUCTIONS:

This program can run on NetBeans IDE on Solaris.

If you use NetBeans, you can open project wc and then open main.c to execute.
Then input characters in the output area. If you want to count number of words,
lines and characters of one or more files, you should first change and add the 
path parameter from properties->run->OUTPUT_PATH.

If you use terminal and vim. You can gmake from wc directory. Then run the
executable binary file directly.

If you do not select counting options, the default counting options are -l,-w,-c
If you do have selections, please make sure -c and -C, -m and -c do not occur at
the same time, just as wc command in linux/solaris does. Otherwise, an error will
occur and the error information is just like wc command in solaris.

The input can be standard input, one file or more files. If there are more file,
the total number of your selected options will also be counted. If a none-existing
file is inputed, an error information will be given like wc command in solaris does.

If you count bytes, lines and words of a folder, the output will be 0 as wc does.




CHANGELOG:

Can be dated back from the svn repository.



NEED TO BE IMPROVED:

Largefile (>2GBytes) counted by wc has not been tested.
