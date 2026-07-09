#ifndef HANDLERS_H
#define HANDLERS_H

#include <string>
using namespace std;
//1.
std::string ResponseString(std::string &client_message);
//2.
std::string UserAgentHeader(std::string &client_message);
//3.
void concurrentConnections(int client_socket);
void handleclient(int client_socket);
void handleFile(int client_socket,string tar_dir);
void POSTready(int client_socket,string dir);
#endif 