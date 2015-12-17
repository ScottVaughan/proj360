#pragma once

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <string>

#include "message.h"

using namespace std;

class Server {
public:
    Server(int port);
    ~Server();

    void run();
    
private:
    void create();
    void close_socket();
    void serve();
    void handle(int);
    
    string read_message(string);
    string parse_message(string, int);
    string read_put(int, int);
    void store_message(string, string, string);
    
    string get_request(int);
    string get_subjects(string);
    string get_message(string, int);
    
    bool send_response(int, string);
    
    map <string, vector<vector<string> > > messagesMap;

    int port_;
    int server_;
    int buflen_;
    char* buf_;
    
    string cache;
};
