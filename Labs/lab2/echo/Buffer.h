#include <pthread.h>
#include <queue>          // std::queue
#include <iostream>

#pragma once

using namespace std;

class Buffer{
	
	public:
		Buffer();
		~Buffer();
		
		void append(int);
		int take();
		
	private:
		pthread_mutex_t lock;
		pthread_cond_t not_empty;
		std::queue<int> bufferQueue;
};
