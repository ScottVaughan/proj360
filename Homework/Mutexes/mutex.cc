
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>

using namespace std;

struct count {
    int counter;
    pthread_mutex_t mutex;
};

void *store(void *);
void *read(void *);

int
main(int argc, char **argv)
{
    pthread_t tidA, tidB;
    // counter in shared memory
    struct count c;

    // initialize mutex
    pthread_mutex_init(&c.mutex, NULL);

    c.counter = 0;
    srandom(1000);

    // create two threads
    pthread_create(&tidA, NULL, &store, &c);
    pthread_create(&tidB, NULL, &read, &c);

    // wait for both threads to terminate
    pthread_join(tidA, NULL);
    pthread_join(tidB, NULL);
  
    exit(0);
}

void *
store(void *vptr)
{
    int val;
    struct count* c;

    c = (struct count*) vptr;

	pthread_mutex_lock(&c->mutex);

	srand (time(NULL));
	val = rand() % 100 + 1;		//val in the range 1 to 100

	cout << pthread_self() << " " << " Storing: "<< val << endl;
	c->counter = val;
	pthread_mutex_unlock(&c->mutex);
}

void *
read(void *vptr){
	struct count* c;
	long r;
    c = (struct count*) vptr;
    
    pthread_mutex_lock(&c->mutex);
    r = random() % 100;
    usleep(r);
	cout << pthread_self() << " " << " Reading: " << c->counter << endl;
	r = random() % 100;
    usleep(r);
    pthread_mutex_unlock(&c->mutex);
}
