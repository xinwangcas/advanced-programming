#include <iostream>
#include <sys/stat.h>
#include <cstring>
#include <dirent.h>
#include <vector>
#include <regex.h>
#include <fstream>
#include <algorithm>
#include <set>
#include <ctime>
#include <map>
#include <mpicxx.h>

using namespace std;
using namespace __gnu_cxx;

//define the parameters of getopt()
extern int optind; 
extern int opterr; 
extern int optopt;

//Define a class WordData which is the value of the map, the key of _map is the words of the files
//In WordData we can store the word, the frequency of the word and which file the word belongs to

class WordData
{
public:
	string word;//the word in files, split by permutation
	int count;//number of each word
	string filename;//the name of the file each word belongs to
	WordData()  {}//constructor of WordData
	~WordData() {}//desctructor of WordData
};

//compare two strings, if they are equal to each other, return 1, else return 0

struct compare_string
{
	bool operator()(const string &p1, const string &p2) const
	{
		return strcmp(p1.c_str(),p2.c_str()) == 0;
	}
};

//retrieve the values of the map according to the key value pairs

struct RetriveValues
{
	template <typename T>
	typename T::second_type operator()(T keyvaluepair) const
	{
		return keyvaluepair.second;
	}
};

//compare the words in map for output file, compare the words according to the frequency and alphabetical order
//
//
struct sort_word
{
	bool operator()(const WordData &wd1, const WordData &wd2) const
	{
		if (wd1.count < wd2.count)//sort the words according to frequency
			return 0;
		else if (wd1.count == wd2.count)
		{
			if(strcmp(wd1.word.c_str(), wd2.word.c_str())>0)//when two words have the same frequency, sort them according to alphabetical order
				return 0;
			else 
				return 1;
		}
		else 
			return 1;
	}
};

//in class TextConcord, a map which stores the words in the files and some attributes of words, this class
//contains two functions, one is to add words into the map, the other is to output results.It also defines a map
//which stores words appear in the files and values of the words.

class TextConcord{
public:
	map<string, WordData>  concord;
	int N;
public:
	//constructor of TextConcord
	TextConcord(int tmpN = 3) :  N(tmpN){
	}
	//destructor of TextConcord, N is the parameter of selection -n. the default value is 3.
	~TextConcord(){
	}
	
	void addWord(const char *, const char * filename);
	string output();

};

//output words in the map and write the ordered strings in a file whose name contains the system time


string TextConcord::output()
{
	vector <WordData> words;//In output construct a vector, which stores all the words

	transform(concord.begin(), concord.end(), back_inserter(words), RetriveValues());
	
	vector <WordData>::iterator wptr;//iterator wptr traverse every word in the vector.
	set <WordData, sort_word> words_in_set;//construct a set to store words and sort the words without repitation.
	set <WordData, sort_word>::iterator sptr;
	for(wptr = words.begin(); wptr != words.end(); wptr++)//If the word is not in the set, add the word in.
		words_in_set.insert(*wptr);	
	
	string result("");

	for (sptr = words_in_set.begin() ;sptr != words_in_set.end(); sptr ++)
	{	
		if(this->N--)//output the most frequent n words in each file.The output result include filename,most frequent words and number of times they occur.
		{
			char num_string[64];

			sprintf(num_string, "%d", sptr->count);
			result += sptr->filename;
			result += ",";
			result += sptr->word;
			result += ",";
			result += num_string;
			result += '\n';
		}
		else
			break;
	}

	return result;	
}

//parse files in the directory and split the words in the files
string parse(char * filename, int N)
{
	TextConcord *tc = new TextConcord(N);
	
	fstream freader(filename,ios::in);//open a file freader to read contents from the input files
	string line;
	char * word_ptr;//word pointer
	char * cstr;
	while(getline(freader, line)){
		  cstr = new char [line.size()+1];
		  strcpy (cstr, line.c_str());

		for(	word_ptr = strtok(cstr,"\n\t .?!:;-()[]{}'\",/"); //split words according to defination of permutation
			word_ptr; 
			word_ptr = strtok(NULL,"\n\t .?!:;-()[]{}'\",/"))//split every word until the end
		{
			tc->addWord(word_ptr, filename);
		}
		delete [] cstr;
	}

	freader.close();
	
	string result("");
	result = tc->output();//output the result
	delete tc;

	return result;
}

//after parsing the file, add the words from the files to the map as the values, the values of map include words,
//count of words and filename
void TextConcord::addWord(const char * word, const char * filename)
{
	if (strcmp(word, "") == 0)
		return;
	

	string sword(word);//change word of type char * to type string
	transform(sword.begin(),sword.end(),sword.begin(),::tolower);//transform capital letters to lower_case letters of each word.

	map<string, WordData> :: iterator map_ptr1;//define a map pointer
	map_ptr1 = concord.find(sword);
	
	if(map_ptr1 != concord.end())//if the newly read word already exists, count = count + 1
	{
		WordData & data = concord[sword];
		data.count ++;
	}
	else//if the newly read word does not exist in map concord, put it in the map.
	{
		WordData * data = new WordData();
		data->count = 1;
		data->word = sword;
		data->filename = string(filename);

		concord[sword] = *data;
	}
		
}

int main(int argc, char * argv[]){

	int tag, send_tag;//tag in MPI_Recv
        int to,from;//destination and source of MPI send/receive
	int st_count, st_source, st_tag;
	double start_time = 0.0;//set start and end time for MPI_Wtime()
	double end_time = 0.0;
	MPI::Status status;

	MPI::Init(argc, argv);//start MPI
	int rank = MPI::COMM_WORLD.Get_rank();//The rank label of the machines
	int size = MPI::COMM_WORLD.Get_size();//The number of tasks to be done
//	MPI_Barrier(MPI_COMM_WORLD);
	int option;

	opterr = 0;
	int N = 0;
	string directory;

	while ((option = getopt(argc, argv, "d:n:"))!= -1)//getopt parses the parameters of commands, -n is the first n words that occur most frequently in files, -d is the directory which contains the files that need to be parsed.
	{
		switch (option)
		{
			case 'n':
				N = atoi(optarg);//the first N words
			break;
			case 'd':
				directory = string(optarg);// parameter of the directory
//				cout << dir <<endl;
			break;
			case '?'://when the parameter of option n is wrong, show the error information
				if (optopt == 'n')
					cerr<< "Option -"<<char(optopt)<<" requires an argument." <<endl;
			        else if (isprint (optopt))
					cerr<< "Unknown option `-"<<char(optopt)<<"'.\n"<<endl;
				else
					cerr<<  "Unknown option character `"<<std::hex<<optopt<<"'."<<endl;
		}
	}

	vector<string> filenames;//use this vector to store file names
	char buffer[1024];
	
	if(rank == 0)//Machine 0 parses the name of directory and files in the directory.
	{	
		struct dirent *ptr;
		DIR *dir;
		dir = opendir(directory.c_str());//open the directory

		while((ptr = readdir(dir))!=NULL)//read the name of the directory
		{
			if(ptr->d_name[0]=='.')
				continue;
			strcpy(buffer,directory.c_str());
			strcat(buffer,ptr->d_name);
//			cout<<buffer<<endl;
			
			filenames.push_back(string(buffer));//put the file names of the directory in the vector filenames
		};
	}

	if(rank == 0)//machine 0 send messages and assign tasks to all the machines, including itself.
	{
		start_time = MPI_Wtime();//star time stamp
		to = 0;
		send_tag = 0;
		int round = 0;

		while(round * size < filenames.size())
		{
			for(int i = round * size; i < (round + 1) * size && i < filenames.size(); i++)
			{
				sprintf(buffer, "%s", filenames[i].c_str());
				
//				cout << rank << ":"<< "sending " << buffer << endl;
				MPI::COMM_WORLD.Send(buffer,1024, MPI::CHAR, i%size, send_tag);//send filenames to the other machines and let them parse the files, including itself.
				to++;
				send_tag++;
			}

		
			tag = MPI::ANY_TAG;
			from = MPI::ANY_SOURCE;
			MPI::COMM_WORLD.Recv(buffer, 1024, MPI::CHAR, from, tag, status);//rank 0 receive parsing result from the rest machines, including itself
			st_count = status.Get_count(MPI::CHAR);
			st_source = status.Get_source();
			st_tag = status.Get_tag();
			
			string result("");
			result = parse(buffer, N);
			strcpy(buffer,result.c_str());

			MPI::COMM_WORLD.Send(buffer,1024, MPI::CHAR, 0, st_tag);//rank 0 send message to itself
			
			for(int i = round * size; i < (round + 1) * size && i < filenames.size(); i++)
			{
				tag = MPI::ANY_TAG;
				from = MPI::ANY_SOURCE;
				MPI::COMM_WORLD.Recv(buffer, 1024, MPI::CHAR, from, tag, status);
				st_count = status.Get_count(MPI::CHAR);
				st_source = status.Get_source();
				st_tag = status.Get_tag();

//				cout << rank <<":" << "received from "<<st_source<<endl<< buffer << endl;
				cout << buffer << endl;
			}

			round++;
		}

		for (int i = 1; i < size; ++i)
		{
			strcpy(buffer, "Finish");
			MPI::COMM_WORLD.Send(buffer,1024, MPI::CHAR, i, 0);//rank 0 send Finish information to the other machines
		}
		end_time = MPI_Wtime();
		printf("The running time is : %lf \n",end_time-start_time);
	}
	else
	{
		while(1)
		{
			tag = MPI::ANY_TAG;
			from = MPI::ANY_SOURCE;
			MPI::COMM_WORLD.Recv(buffer, 1024, MPI::CHAR, from, tag, status);//receive end information from rank 0
			st_count = status.Get_count(MPI::CHAR);
			st_source = status.Get_source();
			st_tag = status.Get_tag();
//			cout<<" rank " << rank <<": " << "st_count:"<<st_count<<" st_source"<< st_source << " st_tag "<< st_tag << endl;
//			cout<<"         " << buffer <<endl;

			if (strcmp(buffer, "Finish") == 0)//if the machine receives the finish information, stop receive and send
				break;

			string result("");
			result = parse(buffer, N);//parse the file received from rank 0
			strcpy(buffer,result.c_str());

			MPI::COMM_WORLD.Send(buffer,1024, MPI::CHAR, 0, st_tag);//send information back to rank 0
		}
	}

//	cout << "rank " << rank <<": "<<"I am dying, goodbye!"<<endl;
//	MPI_Barrier(MPI_COMM_WORLD);
	MPI::Finalize();//MPI finalize
	return 0;
}
