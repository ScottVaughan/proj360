#include "server.h"
//#define NUM_THREADS 10

Server::Server(int port) {
    // setup variables
    port_ = port;
    buflen_ = 1024;
    buf_ = new char[buflen_+1];
    pthread_mutex_init(&lock, NULL);
}

Server::~Server() {
    delete buf_;
}

void
Server::run() {
	
    // create and run the server
    create();

	int NUM_THREADS = 10;
    pthread_t threads[NUM_THREADS];
	for(int i = 0; i < NUM_THREADS; i++){
		pthread_create(&threads[i], NULL, &Server::thread, this);
	}
			
    for(int i = 0; i < NUM_THREADS; i++){
		pthread_join(threads[i], NULL);
	}
}

void
Server::create() {
    struct sockaddr_in server_addr;

    // setup socket address structure
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // create socket
    server_ = socket(PF_INET,SOCK_STREAM,0);
    if (!server_) {
        perror("socket");
        exit(-1);
    }

    // set socket to immediately reuse port when the application closes
    int reuse = 1;
    if (setsockopt(server_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        exit(-1);
    }

    // call bind to associate the socket with our local address and
    // port
    if (bind(server_,(const struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) {
        perror("bind");
        exit(-1);
    }

    // convert the socket to listen for incoming connections
    if (listen(server_,SOMAXCONN) < 0) {
        perror("listen");
        exit(-1);
    }
}

void
Server::close_socket() {
    close(server_);
}

void *
Server::thread(void *vptr){
	Server* server;
	server = (Server*) vptr;
    server->serve();
}

void
Server::serve() {
	
    // setup client
    int client;
    struct sockaddr_in client_addr;
    socklen_t clientlen = sizeof(client_addr);
   
    // accept clients
    while ((client = accept(server_,(struct sockaddr *)&client_addr,&clientlen)) > 0) {
		cout<< "Accepted Cleint: "<< client << "  Thread: "<< pthread_self() <<endl;
		buffer.append(client);	
		pthread_mutex_lock(&lock);
		handle();
		pthread_mutex_unlock(&lock);
    }
    close_socket();
}

void
Server::handle() {
	
    // loop to handle all requests
    while (1) {
		
		//take one client off at a time
		int client = buffer.take();

        // get a request
        string request = get_request(client);
		cout<<"Request: "<<request<<endl;
        // break if client is done or an error occurred
        cache += request;
        if (request.empty() || request == "")
            break;
        string command = read_message(request);
		
		
        string response = parse_message(command, client);
		cout<<"Response: "<<response<<endl;
        // send response
        bool success = send_response(client,response);
	
        // break if an error occurred
        if (not success)
            break;
        //append client back on
		buffer.append(client);
		
        //close(client);
    }
}

string
Server::read_message(string request){
	string message;
	int num = request.find("\n");
    if(num == -1)
        return "";
    else{
        message = request.substr(0,num+1);
        cache = request.substr(num+1,cache.size());
        return message; 
    }
}
string
Server::parse_message(string request, int client){

	string user;
	string subject;
	int length;
	int index;
	string messageContent;
	
	string command;
	std::istringstream iss(request);
	iss >> command;
	
	if(iss.fail())
		return "error invalid message\n";
	else if(command == "reset"){
		//map <string, vector<vector<string> > > messagesMap = messagemap.getMessageMap();
		messagesMap.clear();
		//messagemap.setMessageMap(messagesMap);
		return "OK\n";
	}
	else if(command == "put"){

		iss >> user;
		iss >> subject;
		iss >> length;

		if(iss.fail())
			return "error invalid message\n";
		string data = read_put(length, client);
		
		if (data == "")
			return "error could not read entire message";

		store_message(user, subject, data);
		return "OK\n";
	}
	else if(command == "list"){
		iss >> user;
		if(iss.fail())
			return "error invalid message\n";
		string response = get_subjects(user);
		return response; 
	}
	else if(command == "get"){
		
		iss >> user;
		iss >> index;
	
		if(iss.fail())
			return "error invalid message\n";
			
		string data = get_message(user, index);
	
        if(data == "")
            return "error no such message for that user\n";
        return data;
	}
	return "error invalid message\n";
}

/***command put ***/
string 
Server::read_put(int length, int client){
	string data = cache;

	while(data.length() < length){
		memset(buf_,0,buflen_+1);//resets cache to 0
        int nread = recv(client,buf_,1024,0);
        string d = buf_;
		data += d;
	}
	if(data.length() > length){
		cout<<"datalen"<<data.length()<<endl;
		cache = data.substr(length, data.length());
		data = data.substr(0,length);
	}
	else
		cache = "";
	return data;
}

void
Server::store_message(string user, string subject, string data){
	
	vector<vector<string> > messages;
	vector<string> message;
	map <string, vector<vector<string> > >::iterator itor;
	//map <string, vector<vector<string> > > messagesMap = messagemap.getMessageMap();
		
		
	if(messagesMap.find(user) == messagesMap.end() ){
		messagesMap.insert( std::make_pair(user, messages));
	}
	itor =  messagesMap.find(user);
	messages = itor->second ;
	message.push_back(subject);
	message.push_back(data);
	messages.push_back(message);
	itor->second = messages;
	//messagemap.setMessageMap(messagesMap);
}

/*** command list ***/
string
Server::get_subjects(string user){
	
	int length;
	vector<vector<string> > messageList;
	string response = "";
	
	std::ostringstream oss; 
	//map <string, vector<vector<string> > > messagesMap = messagemap.getMessageMap();
	if(messagesMap.find(user) == messagesMap.end()){
			oss<<" 0"<<endl;
			return oss.str();
	}
	messageList = messagesMap.find(user)->second;
	oss<<"list "<<messageList.size()<<endl;
	for(int i = 0; i < messageList.size(); i++)
		oss<<i+1<<" "<<messageList[i][0]<<endl; 
	//messagemap.setMessageMap(messagesMap);
	return oss.str();
}

/*** command get ***/
string
Server::get_message(string user, int index){
    vector<vector<string> > message;
    std::ostringstream oss; 
    
    //map <string, vector<vector<string> > > messagesMap = messagemap.getMessageMap();
    try{
		message = messagesMap.find(user)->second;
		//messagemap.setMessageMap(messagesMap);
	}catch(...){
		return "";
	}

    if(index > 0){
			string subject = message[index - 1][0];
			string messageContent = message[index - 1][1];
			oss<<"message "<<subject<<" "<<messageContent.size()<<endl<<messageContent;
			return oss.str();					
    }
    
    return "";
}

string
Server::get_request(int client) {
	
    string request = "";
    // read until we get a newline
    while (request.find("\n") == string::npos) {
		memset(buf_,0,buflen_+1);//resets cache to 0
        int nread = recv(client,buf_,1024,0);
        
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
        request.append(buf_,nread);
    }
    // a better server would cut off anything after the newline and
    // save it in a cache
    return request;
}

bool
Server::send_response(int client, string response) {
    // prepare to send response
    const char* ptr = response.c_str();
    int nleft = response.length();
    int nwritten;
    // loop to be sure it is all sent
    while (nleft) {
        if ((nwritten = send(client, ptr, nleft, 0)) < 0) {
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


