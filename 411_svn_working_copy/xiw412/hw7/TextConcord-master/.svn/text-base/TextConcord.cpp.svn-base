#include <iostream>
#include <ext/hash_map>
#include <sys/stat.h>
#include <string>
#include <dirent.h>
#include <vector>
#include <regex.h>
#include <fstream>
#include <algorithm>
#include <set>
#include <ctime>

using namespace std;
using namespace __gnu_cxx;

//Define a class WordData which is the value of the hash_map, the key of hash_map is the words of the files

class WordData
{
public:
	string word;
	int count;
	set <string> filenames;
	WordData()  {}
	~WordData() {}
};

//a method in hash map to convert a word to string type
struct hash_string
{
	size_t operator()(const string&  str)const
	{
		return __stl_hash_string(str.c_str());
	}
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
	hash_map<string, WordData, hash_string, compare_string>  concord;
public:
	//constructor of TextConcord
	TextConcord(){
	}
	//destructor of TextConcord
	~TextConcord(){
	}
	
	void doConcordance(char * directory);
	void parse(char * filename);
	void addWord(const char *, const char * filename);
	void output();

};

//output words in the map and write the ordered strings in a file whose name contains the system time
void TextConcord::output()
{
	vector <WordData> words;
	transform(concord.begin(), concord.end(), back_inserter(words), RetriveValues());
	
	vector <WordData>::iterator wptr;
	set <WordData, sort_word> words_in_set;
	set <WordData, sort_word>::iterator sptr;
	for(wptr = words.begin(); wptr != words.end(); wptr++)
	{
//		cout<<wptr->word<<endl;
		words_in_set.insert(*wptr);	
	}

	string output_file_name("concord.");
 
 	char buf[16];
  	snprintf(buf, 16, "%lu", time(NULL));
	output_file_name += string(buf);
	output_file_name += ".out";
//	cout<<output_file_name<<endl;

	fstream fwriter(output_file_name.c_str(), ios::out);
	for (sptr = words_in_set.begin() ;sptr != words_in_set.end(); sptr ++)
		fwriter<<sptr->word<<endl;
	fwriter.close();
}

//read the name of files in the directory, which is the first command line parameter, to buffer and parse the content of buffer in function parse()
void TextConcord::doConcordance(char * directory)
{
	struct dirent *ptr;
	DIR *dir;
	dir = opendir(directory);
	while((ptr = readdir(dir))!=NULL)
	{
		if(ptr->d_name[0]=='.')
			continue;
		char buffer[1024];
		strcpy(buffer,directory);
		strcat(buffer,ptr->d_name);
		//cout<<buffer<<endl;
		parse(buffer);
	};
	output();
}

//parse files in the directory and split the words in the files
void TextConcord::parse(char * filename)
{
	fstream freader(filename,ios::in);
	string line;
	char * word_ptr;
	char * cstr;

	while(getline(freader, line)){
//split words in each line using regular expressions ("[\\s,;.\"()\\[\\]:?!<>{}*]"))
		  cstr = new char [line.size()+1];
		  strcpy (cstr, line.c_str());

		for(	word_ptr = strtok(cstr,"\n\t\x0B\r\f,;.\"()\\:?!<>{}*]"); 
			word_ptr; 
			word_ptr = strtok(NULL,"\n\t\x0B\r\f,;.\"()\\:?!<>{}*]"))
		{
			addWord(word_ptr, filename);
		}
		delete [] cstr;
	}

	freader.close();
}

//after parsing the file, add the words from the files to the hash map as the values, the values of hash map include words,
//count of words and filename
void TextConcord::addWord(const char * word, const char * filename)
{
	if (strcmp(word, "") == 0)
		return;
	string sword(word);
	transform(sword.begin(),sword.end(),sword.begin(),::tolower);

	hash_map<string, WordData, hash_string, compare_string> :: const_iterator hash_ptr1;
	
	hash_ptr1 = concord.find(sword);
	if(hash_ptr1 != concord.end())
	{
		WordData & data = concord.operator[](sword);
		data.count ++;
		data.filenames.insert(string(filename));
	}
	else
	{
		WordData *data = new WordData();
		data->count = 1;
		data->word = sword;
		data->filenames.insert(string(filename));

		concord[sword] = *data;
	}
		
	cout<<concord[sword].word<<concord[sword].count<<endl;
}

int main(int argc, char * argv[]){
	struct stat dir_info;
	stat(argv[1], &dir_info);
	if (!S_ISDIR(dir_info.st_mode)){
		cerr<<"Error: "<<argv[1]<<" is not a directory"<<endl;
		exit(1);
	}
	TextConcord *tc = new TextConcord();
	tc->doConcordance(argv[1]);
	cout<<"Did concordance"<<endl;
	return 0;
}
