#include <stdarg.h>
#include <resolv.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <resolv.h>
#include <sstream>
#include <fstream>
#include <arpa/inet.h>
#include <string>
#include <vector>
#include <fstream>
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <pthread.h>
using namespace std;
#define PORT 8080
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
int new_socket;
char buffer[100000]={0};
using namespace std::chrono_literals;

long f(){
	return read(new_socket, buffer,100000);
}

long f_wrapper()
{
    std::mutex m;
    std::condition_variable cv;
    long retValue;

    std::thread t([&cv, &retValue]() 
    {
        retValue = f();
        cv.notify_one();
    });

    t.detach();

    {
        std::unique_lock<std::mutex> l(m);
        if(cv.wait_for(l, 10s) == std::cv_status::timeout) 
            throw std::runtime_error("Timeout");
    }

    return retValue;    
}
void SplitString(string s, vector<string> &request){
    std::size_t to = s.find_first_of("\r");
    s=s.substr(0,to);
	string temp="";
    for(int i=0;i<s.length();++i){

        if(s[i]==' '){
            request.push_back(temp);
            temp = "";
        }
        else{
            temp.push_back(s[i]);
        }

    }
    request.push_back(temp);
}
string filetypeIdentifer(string fileName){
    string type="";
    if (fileName.find(".txt") != std::string::npos) {
        type="text/plain";
    }else if(fileName.find(".html") != std::string::npos) {
        type="text/html";
    }else if (fileName.find(".jpg") != std::string::npos) {
		type = "image/jpg\r\n";
	}
	else if (fileName.find(".png") != std::string::npos) {
		type = "image/png\r\n";
	}
	else if (fileName.find(".jpeg") != std::string::npos) {
		type = "image/jpeg\r\n";
	}
    return type;
}
string readTheFile(string fileName) {
	string type =filetypeIdentifer(fileName);
	string line;
    std::ifstream myfile(fileName);
	string fileRead="";
	if (type == "text/plain"||type=="text/html") {
		//read text and html files
		if (myfile.is_open()) {
		    fileRead += ("HTTP/1.0 200 OK \r\n");
            fileRead += ("Content-Type : " + type + "\r\n");
			fileRead += "{";
			while (myfile.good()) {
				//line
				getline(myfile, line);
				fileRead += (line);
				fileRead += "\n";
			}
			fileRead += "}";
			myfile.close();
		}
		else {
			return "";
		}
	} else {
			long size = 0;
			fileRead += ("HTTP/1.0 200 OK \r\n");
			fileRead += ("Content-Type : " + type + "\r\n");
			ifstream fin(fileName ,ios::in | ios::binary);
			if(fin){
				ostringstream oss;
				oss << fin.rdbuf();
				fileRead += "{";
				fileRead +=oss.str();
				fileRead += "}";
			}else {
			return "";
			}

		}
	return fileRead;
}
void write_file(string file_Name,string response ){

        size_t file_start = response.find("{", 0);
		size_t file_end = response.length() - 1;
		int file_length = file_end - file_start - 1;
        string file = response.substr(file_start + 1, file_length);
		std::ofstream writefile(file_Name.c_str(), ios::binary);
        writefile.write(file.c_str(), file.length());
		writefile.close();
}
int main(int argc, char const *argv[])
{
    int server_fd,new_socket; long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }


    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    while(1)
    {
		printf("\n+++++++ Waiting for new connection ++++++++\n\n");
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
		{
			perror("In accept");
			exit(EXIT_FAILURE);
		}else{
			printf("Socket connected:%d",new_socket);
		}
		pid_t PID = fork();
		if (PID == 0) {
			while (true)
			{   
				 buffer[100000] = {0};
				printf("\n+++++++ Waiting for new request ++++++++\n\n");
				//valread = read(new_socket, buffer,100000);
				try
				{
					f_wrapper();
				}
				catch (std::runtime_error &e)
				{
					std::cout << e.what() << std::endl;
					break;
				}
				vector<string> request;
				SplitString(buffer, request);
				if (buffer[1] == 'r')
					break;
				string method = request[0];
				string respose;
				string filePath = request[1].substr(1, request[1].length());
				if (method == "GET")
				{
                    printf("path: %s",filePath.c_str());
					respose = readTheFile(filePath);
					if (respose.length() == 0)
					{
						respose = "HTTP/1.0 400 not found";
					}
				}
				else
				{
					write_file(filePath, buffer);
					respose = "HTTP/1.0 200 OK";
				}
			char* reply=new char[respose.length() + 1];
			strcpy(reply,respose.c_str());
			printf("%s\n",reply);
			write(new_socket ,reply, strlen(reply));
			printf("\n------------------server response sent-------------------\n");
		   }
		   string response="Connection Ended";
		   char* reply=new char[response.length() + 1];
		   strcpy(reply,response.c_str());
		   write(new_socket ,reply, strlen(reply));
		   printf("\n----------connection ended---------------------------\n");
		   close(new_socket);
		   exit(0);
		}else{
			close(new_socket);
		}
    }
    return 0;
}