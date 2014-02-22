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

extern int optind; 
extern int opterr; 
extern int optopt;
//Define a class WordData which is the value of the map, the key of _map is the words of the files

class WordData
{
public:
	string word;
	int count;
	string filename;
	WordData()  {}
	~WordData() {}
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

//compare the words in hash map for output file, compare the words according to the frequency and alphabetical order
struct sort_word
{
	bool operator()(const WordData &wd1, const WordData &wd2) const
	{
		if (wd1.count < wd2.count)
			return 0;
		else if (wd1.count == wd2.count)
		{
			if(strcmp(wd1.word.c_str(), wd2.word.c_str())>0)
				return 0;
			else 
				return 1;
		}
		else 
			return 1;
	}
};

//in class TextConcord, a map which stores the words in the files and some attributes of words
class TextConcord{
public:
	map<string, WordData>  concord;
	int N;
public:
	//constructor of TextConcord
	TextConcord(int tmpN = 3) :  N(tmpN){
	}
	//destructor of TextConcord
	~TextConcord(){
	}
	
	void addWord(const char *, const char * filename);
	string output();

};

//output words in the map and write the ordered strings in a file whose name contains the system time
string TextConcord::output()
{
	vector <WordData> words;

	transform(concord.begin(), concord.end(), back_inserter(words), RetriveValues());
	
	vector <WordData>::iterator wptr;
	set <WordData, sort_word> words_in_set;
	set <WordData, sort_word>::iterator sptr;
	for(wptr = words.begin(); wptr != words.end(); wptr++)
		words_in_set.insert(*wptr);	
	
	string result("");

	for (sptr = words_in_set.begin() ;sptr != words_in_set.end(); sptr ++)
	{	
		if(this->N--)
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
	
	fstream freader(filename,ios::in);
	string line;
	char * word_ptr;
	char * cstr;
	while(getline(freader, line)){
//split words in each line using regular expressions ("[\\s,;.\"()\\[\\]:?!<>{}*]"))
		  cstr = new char [line.size()+1];
		  strcpy (cstr, line.c_str());

		for(	word_ptr = strtok(cstr,"\n\t .?!:;-()[]{}'\",/"); 
			word_ptr; 
			word_ptr = strtok(NULL,"\n\t .?!:;-()[]{}'\",/"))
		{
			tc->addWord(word_ptr, filename);
		}
		delete [] cstr;
	}

	freader.close();
	
	string result("");
	result = tc->output();
	delete tc;

	return result;
}

//after parsing the file, add the words from the files to the hash map as the values, the values of hash map include words,
//count of words and filename
void TextConcord::addWord(const char * word, const char * filename)
{
	if (strcmp(word, "") == 0)
		return;
	

	string sword(word);
	transform(sword.begin(),sword.end(),sword.begin(),::tolower);

	map<string, WordData> :: iterator map_ptr1;
	map_ptr1 = concord.find(sword);
	
//	cout<<sword<<endl;

	if(map_ptr1 != concord.end())
	{
//		cout << " dddd  "<< endl;
		WordData & data = concord[sword];
		data.count ++;
	}
	else
	{
//		cout<<"else"<<endl;
		WordData * data = new WordData();
		data->count = 1;
		data->word = sword;
		data->filename = string(filename);

		concord[sword] = *data;
	}
		
//	map<string, WordData, compare_string> :: iterator mapi;	
	
//	cout<<concord.size()<<endl;
//	for(mapi = concord.begin(); mapi != concord.end(); mapi++)
//		cout << (*mapi).second.word<< (*mapi).second.count<<" ";
//	cout<<endl;
}

int main(int argc, char * argv[]){

	int tag, send_tag;
        int to,from;
	int st_count, st_source, st_tag;
	MPI::Status status;

	MPI::Init(argc, argv);
	int rank = MPI::COMM_WORLD.Get_rank();
	int size = MPI::COMM_WORLD.Get_size();
	int option;

	opterr = 0;
	int N = 0;
	string directory;

	while ((option = getopt(argc, argv, "d:n:"))!= -1)
	{
		switch (option)
		{
			case 'n':
				N = atoi(optarg);
			break;
			case 'd':
				directory = string(optarg);
//				cout << dir <<endl;
			break;
			case '?':
				if (optopt == 'n')
					cerr<< "Option -"<<char(optopt)<<" requires an argument." <<endl;
			        else if (isprint (optopt))
					cerr<< "Unknown option `-"<<char(optopt)<<"'.\n"<<endl;
				else
					cerr<<  "Unknown option character `"<<std::hex<<optopt<<"'."<<endl;
		}
	}

	vector<string> filenames;
	char buffer[1024];
	
	if(rank == 0)
	{
		struct dirent *ptr;
		DIR *dir;
		dir = opendir(directory.c_str());

		while((ptr = readdir(dir))!=NULL)
		{
			if(ptr->d_name[0]=='.')
				continue;
			strcpy(buffer,directory.c_str());
			strcat(buffer,ptr->d_name);
			cout<<buffer<<endl;
			
			filenames.push_back(string(buffer));
		};
	}

	if(rank == 0)
	{
		to = 0;
		send_tag = 0;
		int round = 0;

		for(int i = 0; i < filenames.size(); i++)
		{
			sprintf(buffer, "%s", filenames[i].c_str());
			
			cout << rank << ":"<< "sending " << buffer << endl;

			MPI::COMM_WORLD.Send(buffer,1024, MPI::CHAR, i%size, send_tag);
			to++;
			send_tag++;
	
			if(i % size == 0)
			{
				tag = MPI::ANY_TAG;
				from = MPI::ANY_SOURCE;
				MPI::COMM_WORLD.Recv(buffer, 1024, MPI::CHAR, from, tag, status);
				st_count = status.Get_count(MPI::CHAR);
				st_source = status.Get_source();
				st_tag = status.Get_tag();
				
				string result("");
				result = parse(buffer, N);
				strcpy(buffer,result.c_str());

				MPI::COMM_WORLD.Send(buffer,1024, MPI::CHAR, 0, st_tag);
			}

			
			tag = MPI::ANY_TAG;
			from = MPI::ANY_SOURCE;
			MPI::COMM_WORLD.Recv(buffer, 1024, MPI::CHAR, from, tag, status);
			st_count = status.Get_count(MPI::CHAR);
			st_source = status.Get_source();
			st_tag = status.Get_tag();

			cout << rank <<":" << "received from "<<st_source<<endl<< buffer << endl;
		}

		for (int i = 1; i < size; ++i)
		{
			strcpy(buffer, "Finish");
			MPI::COMM_WORLD.Send(buffer,1024, MPI::CHAR, i, 0);
		}
	}
	else
	{
		while(1)
		{
			tag = MPI::ANY_TAG;
			from = MPI::ANY_SOURCE;
			MPI::COMM_WORLD.Recv(buffer, 1024, MPI::CHAR, from, tag, status);
			st_count = status.Get_count(MPI::CHAR);
			st_source = status.Get_source();
			st_tag = status.Get_tag();
//			cout<<" rank " << rank <<": " << "st_count:"<<st_count<<" st_source"<< st_source << " st_tag "<< st_tag << endl;
//			cout<<"         " << buffer <<endl;

			if (strcmp(buffer, "Finish") == 0)
				break;

			string result("");
			result = parse(buffer, N);
			strcpy(buffer,result.c_str());

			MPI::COMM_WORLD.Send(buffer,1024, MPI::CHAR, 0, st_tag);
		}
	}

	cout << "rank " << rank <<": "<<"I am dying, goodbye!"<<endl;
	MPI::Finalize();
	return 0;
}
