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
using namespace std;

#define MaxBuf 1024 // max buffer size
string fileName = "clientRequest.txt"; // file containing requests
static string portNumber = "8080"; // default port number
string server_ip = "127.0.0.1"; //default host name
void SplitString(string s, vector<string> &request){

    string temp = "";
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
string send_Request(int sock,char* message){
    long valread;
    char buffer[MaxBuf] = {0};
    send(sock , message ,strlen(message), 0 );
    valread = read( sock , buffer, MaxBuf);
    return buffer;
}
void write_file(string file_Name,string response ){

    //writing the response file to the current dictionary     
        size_t file_start = response.find("{", 0);
		size_t file_end = response.length() - 1;
		int file_length = file_end - file_start - 1;
        string file = response.substr(file_start + 1, file_length);
		std::ofstream writefile(file_Name.c_str(), ios::binary);
        writefile.write(file.c_str(), file.length());
		writefile.close();
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
string readTheFile(string requestbody,string fileName) {
	string type =filetypeIdentifer(fileName);
	string line;
    std::ifstream myfile(fileName);
	string fileRead;
	if (type == "text/plain"||type=="text/html") {
		//read text and html files
		if (myfile.is_open()) {
			fileRead += (requestbody + "\r\n");
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
			fileRead += (requestbody +"\r\n");
			fileRead += ("Content-Type : " + type + "\r\n");
			std::ifstream fin(fileName ,std::ios::in | std::ios::binary);
			if(fin){
				std::ostringstream oss;
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
int main(int argc, char const *argv[])
{


    string request_line;

    if(argc <3)
    {
     perror("Error:uncomplete Input");
     exit(EXIT_FAILURE);
    }
    server_ip=argv[1];
    portNumber=argv[2];
    std::ifstream readfile(fileName);
    int sock = 0;
    if(readfile.is_open())
    {
        int count=0;
      while(readfile.good())
       {
           string host_name;
           string port_num;
           getline(readfile,request_line);
           if (strcmp(request_line.c_str(), "\\r\\n") == 0) {
                char* s=new char[request_line.length() + 1];
                strcpy(s,request_line.c_str());
				send_Request(sock,s);
                close(sock);
                break;
			}
            std::size_t to = request_line.find_first_of("\\");
            request_line=request_line.substr(0,to);
            vector<string> request;
            SplitString(request_line,request);
            if(request.size()==4){
                host_name=request[2];
                port_num=request[3];
            } else if(request.size()==3){
                host_name=request[2];
                port_num=portNumber;
            }else if(request.size()==2){
               host_name=server_ip;
               port_num=portNumber;
            }
            string file_Name=request[1].substr(1,request[1].length());
            if(count==0){
                long valread;
                struct sockaddr_in serv_addr;
                if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                {
                    printf("\n Socket creation error \n");
                    exit(0);
                }

                memset(&serv_addr, '0', sizeof(serv_addr));

                serv_addr.sin_family = AF_INET;
                serv_addr.sin_port = htons(stoi(portNumber));

                // Convert IPv4 and IPv6 addresses from text to binary form
                if (inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr) <= 0)
                {
                    printf("\nInvalid address/ Address not supported \n");
                    exit(0);
                }

                if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
                {
                    printf("\nConnection Failed \n");
                    exit(0);
                }
            }
            if(request[0]=="GET"){
                char* s=new char[request_line.length() + 1];
                strcpy(s,request_line.c_str());
                printf("request sent :%s socketNUM:%d count:%d\n",request_line.c_str(),sock,count);
                string response=send_Request(sock,s);
                printf("response recived :%s\n",response.c_str());
                if(strcmp(response.c_str(), "HTTP/1.0 404 Not Found") ==0) continue;
                //writing the response file to the current dictionary  
                write_file(file_Name,response);
            }else{
               string request_body=readTheFile(request_line,file_Name);
                if (request_body.length()==0){
                     printf("Error in Reading File:RequestFaild\n");
                     continue;
                } 
                char* s=new char[request_body.length() + 1];
                strcpy(s,request_body.c_str());
                printf("request sent :%s socketNUM:%d count:%d\n",request_line.c_str(),sock,count);
                string response=send_Request(sock,s);  
                printf("response recived :%s\n",response.c_str());
            }
            count++;
        }
    }

    return 0;
}
