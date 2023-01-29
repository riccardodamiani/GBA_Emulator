#ifndef MULTITHREAD_WORK_H
#define MULTITHREAD_WORK_H

#include <vector>
#include "threadHelper.h"

class MultithreadManager {
public:

	MultithreadManager(int threads);
	void startWork(int count, void(*function)(int start_index, int end_index, void* args), void *args);
	void Wait();
	void updateActiveTasks();
	void destroy();
private:

	std::mutex taskMutex;
	std::condition_variable taskCv;
	int _activeTasks;

	int threadCount;
	std::vector <ThreadHelper*> threads;
};

#endif
