#include "threadHelper.h"
#include "multithreadManager.h"
#include <thread>

ThreadHelper::ThreadHelper(MultithreadManager* boss, int helperID){
	this->boss = boss;
	this->helperID = helperID;
	this->_killThread = false;
	this->_waitVariable = false;
	//this->thrMutex.lock();
	this->thr = std::thread::thread([this] {this->workThread();});
	this->thr.detach();
}

//setup the thread to start a job
void ThreadHelper::startWork(int begin, int end, void(*function)(int start_index, int end_index, void* args), void* args) {
	this->startIndex = begin;
	this->endIndex = end;
	this->arguments = args;
	this->_workDone = false;
	this->jobFunction = function;
	{
		std::lock_guard<std::mutex> lk(this->thrMutex);
		this->_waitVariable = true;		//release the mutex lock
	}
	workCv.notify_all();
}

void ThreadHelper::workThread() {
	
	_threadIsAlive = true;
	std::unique_lock<std::mutex> lk(this->thrMutex);
	workCv.wait(lk, [this] {return this->_waitVariable;});		//wait for a job
	lk.unlock();

	while (!this->_killThread) {

		this->jobFunction(this->startIndex, this->endIndex, this->arguments);

		this->_workDone = true;
		this->_waitVariable = false;

		this->boss->updateActiveTasks();		//tells the multithreadWork object that it completed his job
		{
			std::unique_lock<std::mutex> lk(this->thrMutex);
			workCv.wait(lk, [this] {return this->_waitVariable;});		//wait for the next job
			lk.unlock();
		}
		
	}

	_threadIsAlive = false;
}

bool ThreadHelper::workDone() {
	return this->_workDone;
}

void ThreadHelper::killThread() {
	//wait for the 
	this->_killThread = true;
	{
		std::lock_guard<std::mutex> lk(this->thrMutex);
		this->_waitVariable = true;
	}
	workCv.notify_all();

	while (this->_threadIsAlive)volatile int a = 0;
}