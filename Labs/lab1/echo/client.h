#pragma once

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <string>
#include <sstream> 

using namespace std;

class Client {
public:
    Client(string host, int port);
    ~Client();

    void run();

private:
    virtual void create();
    virtual void close_socket();
    void echo();
    bool parse_command(string);
    bool send_request(string);
    
    string get_response();//changed it from a boolean
	
    string get_user_message();
	void send_put(string, string, string);
    void response_to_put(bool);

	void send_list(string);
	void response_to_list();
	
	
	void read_list_response(int);
	
	void response_to_read();
	void send_read(string, int);
	void read_message_response(string, int);
	

	string read_message();

    string host_;
    int port_;
    int server_;
    int buflen_;
    char* buf_;
    string cache;
};
