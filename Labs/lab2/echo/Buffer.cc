#include "Buffer.h"

Buffer::Buffer(){
	// initialize mutex

	pthread_mutex_init(&lock, NULL);
	pthread_cond_init(&not_empty, NULL);
	//pthread_cond_t(&not_full, NULL);
};
Buffer::~Buffer(){}

//Producer
void Buffer::append(int client){
	pthread_mutex_lock(&lock );

	bufferQueue.push(client);

	pthread_cond_signal(&not_empty);
	pthread_mutex_unlock(&lock);
};

//Consumer
int Buffer::take(){
	pthread_mutex_lock(&lock );
	
	while(bufferQueue.empty()){
		pthread_cond_wait(&not_empty, &lock);
	}
	int client = bufferQueue.front();

	//pop off top value
	bufferQueue.pop();

	pthread_mutex_unlock(&lock );
	return client; 
};

