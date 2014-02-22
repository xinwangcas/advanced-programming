#include <fstream>
#include <iostream>
#include <mpicxx.h>
#include <sys/stat.h>
#include <cstring>
#include <dirent.h>
#include <vector>


using namespace std;
extern int optind; 
extern int opterr; 
extern int optopt;

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

	while ((option = getopt(argc, argv, "d:"))!= -1)//parse the directory by -d
	{
		switch (option)
		{
			case 'd':
				directory = string(optarg);
//				cout << dir <<endl;
			break;
			case '?':
				if (optopt == 'd')
					cerr<< "Option -"<<char(optopt)<<" requires an argument." <<endl;
			        else if (isprint (optopt))
					cerr<< "Unknown option `-"<<char(optopt)<<"'.\n"<<endl;
				else
					cerr<<  "Unknown option character `"<<std::hex<<optopt<<"'."<<endl;
		}
	}
	
	system("rm -fr /tmp/xiw412/");//clear all the files in directory /tmp/xiw412 and the directory itself
	system("mkdir /tmp/xiw412/");//biuld a new directory /tmp/xiw412

	char buffer[1024];
	
	struct dirent *ptr;
	DIR *dir;
	dir = opendir(directory.c_str());

	while((ptr = readdir(dir))!=NULL)//read the directory
	{
		if(ptr->d_name[0]=='.')
			continue;//skip the "." in directory
		strcpy(buffer,directory.c_str());
		strcat(buffer,ptr->d_name);
		cout<<buffer<<endl;

		string line;
	
		fstream file(buffer, ios::in);//open file file for read

		strcpy(buffer,"/tmp/xiw412/");//copy the directory path /tmp/xiw412/ to the buffer
		strcat(buffer,ptr->d_name);//copy the file names to the buffer
		fstream output_file(buffer, ios::out);//open file output_file /tmp/xiw412/* for write
		
		while(getline(file, line)){//read from file
			output_file<<line<<endl;//write to output_file
		}
		file.close();
		output_file.close();
	}

	MPI::Finalize();
	return 0;
}
