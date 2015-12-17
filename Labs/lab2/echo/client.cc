#include "client.h"
//Everything should work the way it was specified to the best of my understanding. 
//I've tested it with the my server and client against each other and with the python code.
Client::Client(string host, int port) {
    // setup variables
    host_ = host;
    port_ = port;
    buflen_ = 1024;
    buf_ = new char[buflen_+1];
}

Client::~Client() {
}

void Client::run() {
    // connect to the server and run echo program
    create();
    echo();
}

void
Client::create() {
    struct sockaddr_in server_addr;

    // use DNS to get IP address
    struct hostent *hostEntry;
    hostEntry = gethostbyname(host_.c_str());
    if (!hostEntry) {
        cout << "No such host name: " << host_ << endl;
        exit(-1);
    }

    // setup socket address structure
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    memcpy(&server_addr.sin_addr, hostEntry->h_addr_list[0], hostEntry->h_length);

    // create socket
    server_ = socket(PF_INET,SOCK_STREAM,0);
    if (!server_) {
        perror("socket");
        exit(-1);
    }

    // connect to server
    if (connect(server_,(const struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) {
        perror("connect");
        exit(-1);
    }
}

void
Client::close_socket() {
    close(server_);
}

void
Client::echo() {
    string line;
    bool result;
    while (true) {
		cout<<"% ";
		getline(cin,line);
        line += "\n";
        result = parse_command(line);
        if(result != true)
			cout<<"I don't recognize that command."<<endl;
    }
    close_socket();
}
bool
Client::parse_command(string line){ 
	
	string command;
	string user;
	string subject;
	int index;
	
	std::istringstream iss(line);
	iss>>command;

	if(command == "reset"){
		send_request("reset\n");
		string response = get_response();
		if(response != "OK\n")
			cout<<"Server returned bad message:"<<response<<endl;
		else 
			return true;
	}
	if(command == "send"){
		iss >> user;
		iss >> subject;
		if(iss.fail())
			return false;
			
		string message;
		message = get_user_message();
		send_put(user, subject, message);
		return true;		
	}
	else if (command == "list"){
		iss >> user;
		if(iss.fail())
			return false;
		send_list(user);
        response_to_list();
		return true;
	}
	else if (command == "read"){
		iss >> user;
		iss >> index;
	
		if(iss.fail())
			return false;
		send_read(user, index);
		response_to_read();
		return true;
	}
	else if(command == "quit"){
		exit(0);
	}
	return false; 
}
/***Send a message to the server***/
string
Client::get_user_message(){
	cout<<"- Type your message. End with a blank line -"<<endl;
	string message = "";

	while(true){
		string line;
		getline(cin,line);
		if(line == "")
			break;
		message += line += "\n"; 
	}
	return message;
}

void 
Client::send_put(string user, string subject, string message){
	std::ostringstream oss;
	oss<<"put "<<user<<" "<<subject<<" "<<message.size()<<endl<<message;
	bool success = send_request(oss.str());
	response_to_put(success);	
}

void 
Client::response_to_put(bool success){
	if (success){
		string response = get_response();
		if(response != "OK\n"){
			cout<<"Server returned bad message:"<<response<<endl;
			return;
		}
	}
}


/*** Get list from server***/
void
Client::send_list(string user){
	std::ostringstream oss;
	oss<<"list "<<user<<endl;
	send_request(oss.str());
}

void
Client::response_to_list(){
	string message;
	message = get_response();
	
	std::istringstream iss(message);
	iss >> message;
	
	if(message != "list"){
		cout<<"Server returned bad message:"<<message<<endl;
		return;
	}
	else if(iss.fail()){
		cout<<"Server returned bad message:"<<message<<endl;
	}
	cout<<cache; 
}

/*** Read a message from the server ***/
void
Client::send_read(string user, int index){
	std::ostringstream oss;
	oss<<"get "<<user<<" "<<index<<endl;
	send_request(oss.str());
}

void
Client::response_to_read(){
	string message;
	string command;
	string subject;
	string length;

	message = get_response();
	std::istringstream iss(message);
	
	iss >> command;
	
	if(command != "message"){
		cout<<"Server returned bad message:"<<message<<endl;
		return;
	}

	iss >> subject;
	iss >> length; 
	if(iss.fail()){
		cout<<"Server returned bad message:"<<message<<endl;
		return;
	}
	cout<<cache<<endl;
}

bool
Client::send_request(string request) {
    // prepare to send request
    const char* ptr = request.c_str();
    int nleft = request.length();
    int nwritten;
    // loop to be sure it is all sent
    while (nleft) {
        if ((nwritten = send(server_, ptr, nleft, 0)) < 0) {
            if (errno == EINTR) {
                // the socket call was interrupted -- try again
                continue;
            } else {
                // an error occurred, so break out
                perror("write");
                return false;
            }
        } else if (nwritten == 0) {
            // the socket is closed
            return false;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return true;
}

string
Client::get_response() {

    string response = "";
    // read until we get a newline
    while (response.find("\n") == string::npos) {
        int nread = recv(server_,buf_,1024,0);
        if (nread < 0) {
            if (errno == EINTR)
                // the socket call was interrupted -- try again
                continue;
            else
                // an error occurred, so break out
                return "";
        } else if (nread == 0) {
            // the socket is closed
            return "";
        }
        // be sure to use append in case we have binary data
        response.append(buf_,nread);
        int num = response.find("\n")+1;
        cache = response.substr(num, response.size());
    }
    // a better client would cut off anything after the newline and
    // save it in a cache
    return response;
}
