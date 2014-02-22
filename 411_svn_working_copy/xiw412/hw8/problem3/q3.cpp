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

	while ((option = getopt(argc, argv, "d:"))!= -1)
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
	
	system("rm -fr /tmp/xiw412/");
	system("mkdir /tmp/xiw412/");

	char buffer[1024];
	
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

		string line;
	
		fstream file(buffer, ios::in);

		strcpy(buffer,"/tmp/xiw412/");
		strcat(buffer,ptr->d_name);
		fstream output_file(buffer, ios::out);
		
		while(getline(file, line)){
			output_file<<line<<endl;
		}
		file.close();
		output_file.close();
	}

	MPI::Finalize();
	return 0;
}
