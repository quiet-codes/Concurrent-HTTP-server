#include "handlers.h"
#include <sstream>
#include<iostream>
#include <sys/socket.h> 
#include <unistd.h>
#include<filesystem>
#include <fstream>
using namespace std;
//5.
string ResponseString(string &client_message)
{
  auto getPath =[](const string& req){
    stringstream ss(req);
    string mem,path;
    ss>>mem>>path;
    return path;
  };
  string path = getPath(client_message);
  string response;
  if(path =="/"){
    response = "HTTP/1.1 200 OK\r\n\r\n";
  }
  else if(path.find("/echo/")==0){
    string actual = path.substr(6);
      response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + to_string(actual.size()) + "\r\n\r\n" + actual;
  }
  else response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
return response;
}


//6.Getting the User-Agent header 
string UserAgentHeader(string &client_message)
{
  string key = "User-Agent: ";
  auto pos = client_message.find(key);
  string res;
  if(pos == string::npos)res="";
  else {
    auto st = pos + key.size();
    auto end = client_message.find("\r\n",st);
    res = client_message.substr(st,end-st);
  }
  return res;
}

//7.
void concurrentConnections(int client_socket)
{
    string buffer(1024,'\0');
    ssize_t b = recv(client_socket,(void*)&buffer[0],1024,0);
    if(b >0)
    {
      auto getPath = [](const string& req){
        stringstream ss(req);
        string mem,path;
        ss>>mem>>path;
        return path;
      };
      string path = getPath(buffer);
      cout<<"BUFFER\r\n";
      cout<<buffer<<endl;
      cout<<path<<endl;
      string response;
      if(path =="/"){
    response = "HTTP/1.1 200 OK\r\n\r\n";
  }
  else if(path.find("/echo/")==0){
    
      string actual = path.substr(6);
      response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + to_string(actual.size()) + "\r\n\r\n" + actual;
  }
  else if(path.find("/user-agent")==0)
  {
    string ua =  UserAgentHeader(buffer);
    response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + to_string(ua.size()) + "\r\n\r\n" + ua;
  }
  else response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
  send(client_socket,response.c_str(),response.size(),0);
    }
    close(client_socket);
}

void handleclient(int client_socket)
{
  string buffer(1024,'\0');
  ssize_t b = recv(client_socket,(void*)&buffer[0],1024,0);
  if(b>0)
  {
    auto getPath =[](const string &r)
    {
      stringstream ss(r);
      string mem,path;
      ss>>mem>>path;
      return path;
    };
    string path = getPath(buffer);
    string response;
    if(path=="/"){
      response = "HTTP/1.1 200 OK\r\n\r\n";
    }
    else if(path.find("/echo/")==0){
      string actual = path.substr(6);
      response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + to_string(actual.size()) + "\r\n\r\n" + actual;
    }
    else if(path.find("/user-agent")==0)
    {
      string ua =  UserAgentHeader(buffer);
    response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + to_string(ua.size()) + "\r\n\r\n" + ua;
    }
    else 
    {
      response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    }
    send(client_socket,response.c_str(),response.size(),0);
    close(client_socket);
  }
}
void handleFile(int client_socket,string tar_dir)
{
  if(!tar_dir.empty() && tar_dir.back()!='/'){
    tar_dir +='/';
  }

  string buffer(1024,'\0');
  ssize_t b = recv(client_socket,(void*)&buffer[0],1024,0);
  if(b>0){
    cout<<"Buffer\n"<<buffer<<endl;
    auto getPath =[](const string &r)
    {
      stringstream ss(r);
      string mem,path;
      ss>>mem>>path;
      return path;
    };
    string path = getPath(buffer);
    string response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    if(path=="/"){
      response = "HTTP/1.1 200 OK\r\n\r\n";
    }
    else if(path.find("/echo/")==0){
      string actual = path.substr(6);
      response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + to_string(actual.size()) + "\r\n\r\n" + actual;
    }
    else if(path.find("/user-agent")==0)
    {
      string ua =  UserAgentHeader(buffer);
    response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + to_string(ua.size()) + "\r\n\r\n" + ua;
    }
    else if(path.starts_with("/files/")){
      string filename = path.substr(7);
      string full_path = tar_dir + filename;
      if(std::filesystem::exists(full_path)){
        ifstream file(full_path, std::ios::binary);
        if(file.is_open()){
          stringstream buffer;
          buffer<< file.rdbuf();
          string file_content = buffer.str();
          file.close();
          size_t  file_size = file_content.length();
          response = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: application/octet-stream\r\n"
                               "Content-Length: " + std::to_string(file_size) + "\r\n\r\n"
                               + file_content;
        }
        else response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
      }
      else response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    }
    send(client_socket,response.c_str(),response.size(),0);
    close(client_socket);
  }
  
}

void readRequestBody(int client_socket,string tar_dir)
{
  if(!tar_dir.empty() && tar_dir.back()!='/'){
    tar_dir +='/';
  }

  string buffer(1024,'\0');
  ssize_t b = recv(client_socket,(void*)&buffer[0],1024,0);
  if(b>0){
    cout<<"Buffer\n"<<buffer<<endl;
    auto getPath =[](const string &r)
    {
      stringstream ss(r);
      string mem,path;
      ss>>mem>>path;
      return path;
    };
    string path = getPath(buffer);
    string response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    if(path=="/"){
      response = "HTTP/1.1 200 OK\r\n\r\n";
    }
    else if(path.find("/echo/")==0){
      string actual = path.substr(6);
      response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + to_string(actual.size()) + "\r\n\r\n" + actual;
    }
    else if(path.find("/user-agent")==0)
    {
      string ua =  UserAgentHeader(buffer);
    response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + to_string(ua.size()) + "\r\n\r\n" + ua;
    }
    else if(path.starts_with("/files/")){
      string filename = path.substr(7);
      string full_path = tar_dir + filename;
      if(std::filesystem::exists(full_path)){
        ifstream file(full_path, std::ios::binary);
        if(file.is_open()){
          stringstream buffer;
          buffer<< file.rdbuf();
          string file_content = buffer.str();
          file.close();
          size_t  file_size = file_content.length();
          response = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: application/octet-stream\r\n"
                               "Content-Length: " + std::to_string(file_size) + "\r\n\r\n"
                               + file_content;
        }
        else response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
      }
      else response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    }
    send(client_socket,response.c_str(),response.size(),0);
    close(client_socket);
  }
}

void POSTready(int client_socket,string dir){
  if(!dir.empty() && dir.back()!='/'){
    dir +='/';
  }

  string buffer(1024,'\0');
  ssize_t b = recv(client_socket,(void*)&buffer[0],1024,0);
  if(b>0){
    cout<<"Buffer\n"<<buffer<<endl;
    auto getPath =[](const string &r)
    {
      stringstream ss(r);
      string mem,path;
      ss>>mem>>path;
      return make_pair(mem,path);
    };
    auto p = getPath(buffer);
    string method= p.first;
    string path  = p.second;
    string response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    if(path=="/"){
      response = "HTTP/1.1 200 OK\r\n\r\n";
    }
    else if(path.find("/echo/")==0){
      string actual = path.substr(6);
      response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + to_string(actual.size()) + "\r\n\r\n" + actual;
    }
    else if(path.find("/user-agent")==0)
    {
      string ua =  UserAgentHeader(buffer);
    response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + to_string(ua.size()) + "\r\n\r\n" + ua;
    }
    else if(path.starts_with("/files/")){
      
      string filename = path.substr(7);
      string full_path = dir + filename;
      if(method =="GET"){
      if(std::filesystem::exists(full_path)){
        ifstream file(full_path, std::ios::binary);
        if(file.is_open()){
          stringstream buffer;
          buffer<< file.rdbuf();
          string file_content = buffer.str();
          file.close();
          size_t  file_size = file_content.length();
          response = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: application/octet-stream\r\n"
                               "Content-Length: " + std::to_string(file_size) + "\r\n\r\n"
                               + file_content;
        }
        else response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
      }
      else response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    }
    else if(method=="POST"){
      size_t body_pos= buffer.find("\r\n\r\n");
      if(body_pos!=string::npos)
      {
        size_t cl_pos = buffer.find("Content-Length: ");
        int content_len=0;
        if(cl_pos!=string::npos){
          size_t cl_end = buffer.find("\r\n",cl_pos);
          string cl_str = buffer.substr(cl_pos + 16,cl_end-(cl_pos + 16));
          content_len = stoi(cl_str);
          cout<<content_len<<endl;
        }
        string request_body = buffer.substr(body_pos + 4,content_len);
        std::ofstream outfile(full_path , std::ios::binary);
        if(outfile.is_open()){
          outfile << request_body;
          outfile.close();
          response = "HTTP/1.1 201 Created\r\n\r\n";
        }else{
          response = "HTTP/1.1 500 Internal Server Erro\r\n\r\n";
        }
      }
    }
    }
    send(client_socket,response.c_str(),response.size(),0);
    close(client_socket);
  }
}
