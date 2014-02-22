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

class WordData
{
public:
	string word;
	int count;
	set <string> filenames;
	WordData()  {}
	~WordData() {}
};

struct hash_string
{
	size_t operator()(const string&  str)const
	{
		return __stl_hash_string(str.c_str());
	}
};

struct compare_string
{
	bool operator()(const string &p1, const string &p2) const
	{
		return strcmp(p1.c_str(),p2.c_str()) == 0;
	}
};

struct RetriveValues
{
	template <typename T>
	typename T::second_type operator()(T keyvaluepair) const
	{
		return keyvaluepair.second;
	}
};


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


class TextConcord{
public:
	hash_map<string, WordData, hash_string, compare_string>  concord;
public:
	TextConcord(){
	}
	~TextConcord(){
	}
	
	void doConcordance(char * directory);
	void parse(char * filename);
	void addWord(const char *, const char * filename);
	void output();

};

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

void TextConcord::parse(char * filename)
{
	fstream freader(filename,ios::in);
	string line;
	char * word_ptr;
	char * cstr;

	while(getline(freader, line)){
//		split_regex(words,line,regex("[\\s,;.\"()\\[\\]:?!<>{}*]"));
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
