
#include "multithreadManager.h"
#include "threadHelper.h"
#include <mutex>
#include <condition_variable>


MultithreadManager::MultithreadManager(int threadsCount) {

	this->threadCount = threadsCount;
    this->_activeTasks = 0;

    for (int i = 0; i < threadsCount; i++) {
        threads.push_back(new ThreadHelper(this, i));
    }
}

//setup all the threads for the work and waits for them to finish
void MultithreadManager::startWork(int count, void(*function)(int start_index, int end_index, void* args), void* args) {

    if (count > 0) {
        this->_activeTasks = this->threadCount;
        //split the work between all the threads
        int countPerThread = count / this->threadCount;
        for (int i = 0; i < this->threadCount - 1; i++) {
            this->threads[i]->startWork(i * countPerThread, i * countPerThread + countPerThread, function, args);
        }
        //the last thread receive all the remaining elements. This is needed in case count is not divisible by the thread number
        this->threads[this->threadCount-1]->startWork((this->threadCount - 1) * countPerThread, count, function, args);

        // Wait for all the thread to finish working before returning
        /*std::unique_lock<std::mutex> lk(this->taskMutex);
        taskCv.wait(lk, [this] {return this->_activeTasks == 0;});*/
    }
}

// Wait for all the thread to finish working before returning
void MultithreadManager::Wait() {
    
    std::unique_lock<std::mutex> lk(this->taskMutex);
    taskCv.wait(lk, [this] {return this->_activeTasks == 0;});

}

//update the active tasks count. When it reaches 0 the lock in startWork() is released
void MultithreadManager::updateActiveTasks() {
    {
        std::lock_guard<std::mutex> lk(this->taskMutex);
        this->_activeTasks--;
    }
    taskCv.notify_one();
}


void MultithreadManager::destroy() {
    for (int i = 0; i < this->threadCount; i++) {
        this->threads[i]->killThread();
        delete this->threads[i];
    }
}