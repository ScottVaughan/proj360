#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include "server.h"

using namespace std;

#define NUM_THREADS     10

void *createThreads(void *);

int
main(int argc, char **argv)
{
	int option, port;
	// setup default arguments
    port = 3000;

    // process command line options using getopt()
    // see "man 3 getopt"
    while ((option = getopt(argc,argv,"p:")) != -1) {
        switch (option) {
            case 'p':
                port = atoi(optarg);
                break;
            default:
                cout << "server [-p port]" << endl;
                exit(EXIT_FAILURE);
        }
    }
	
    Server server = Server(port);
    server.run();
  
	exit(0);
}



