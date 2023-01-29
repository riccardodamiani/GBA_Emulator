#ifndef THREAD_WORKER_H
#define THREAD_WORKER_H

#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>

class MultithreadManager;

class ThreadHelper {
public:
	ThreadHelper(MultithreadManager*, int helperID);
	void startWork(int begin, int end, void(*function)(int start_index, int end_index, void* args), void* args);
	bool workDone();
	void killThread();

private:
	std::thread thr;
	void workThread();
	void (*jobFunction)(int start_index, int end_index, void *args);

	std::mutex thrMutex;
	std::condition_variable workCv;
	bool _waitVariable;

	int startIndex;
	int endIndex;
	void* arguments;
	bool _workDone;
	bool _killThread;
	bool _threadIsAlive;

	int helperID;

	MultithreadManager* boss;
};

#endif
