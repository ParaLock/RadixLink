#pragma once

#include <map>
#include <thread>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <deque>
#include <utility>
#include <condition_variable>
#include <list>

#include "Task.h"

struct Worker {

    std::thread              workerThread;
    bool                     isRunning;
    
    std::deque<Task>         work;
    Worker*                  neighbor;

    std::condition_variable  workAvailable;
    std::mutex               lock;
    bool                     isWorkAvailable;
    unsigned int             threadId;

    Worker(unsigned int threadNum, Worker* next) {
        
        neighbor = next;
        isWorkAvailable = true;

        threadId = threadNum;
    }

    ~Worker() {


    }

    std::thread::id getThreadId() {

        return workerThread.get_id();
    }

    void stop() {

        //if(workerThread) {

            isRunning = false;
            isWorkAvailable = true;
            workAvailable.notify_one();

            workerThread.join();
        //}
    }

    void setNext(Worker* next) {

        neighbor = next;
    }

    void start() {

        isRunning = true;
        workerThread    = std::thread(&Worker::exec, this);
        isWorkAvailable = true;
    }

    bool isAvailable() {

        return isWorkAvailable;
    }

    void feed(Task& next) {

        work.push_back(next);
    }

    void trigger() {

        isWorkAvailable = true;
        workAvailable.notify_one();
    }

    void exec() {

        while(isRunning) {
            
            std::unique_lock<std::mutex> lk(lock);
            workAvailable.wait(lk, [this]() { return this->isWorkAvailable; });

            while(work.size() > 0) {

                auto t = work.front();
                work.pop_front();

                t.func();

                //std::cout << "EXEC WITHIN THREAD: " << threadId << std::endl;

                if(t.completionCallback) {

                    t.completionCallback();
                }

            }

            isWorkAvailable = false;

            lk.unlock();

        }
    }
    
};

class TaskExecutor {
private:

    std::map<std::string, std::pair<unsigned int, std::vector<Worker*>>>          
                                            m_workQueues;

    bool m_isActive;

    std::map<std::thread::id, unsigned int> m_threadIdToIndex;
    
public:

    TaskExecutor() {

        m_isActive = true;
    }

    ~TaskExecutor() {

        
    }

    void stop() {

        m_isActive = false;

        for (auto& queue : m_workQueues) {

            for(auto& worker : queue.second.second) {

                std::cout << "Thread Model: Stopping OR Moving worker thread." << std::endl;

                worker->stop();

                delete worker;
            
                std::cout << "Thread Model: Worker Thread Joined." << std::endl;
            
            }
        }
    }

    void addTask(Task task, bool runOnSelf = false) {

        if(m_isActive) {

            if(m_workQueues.find(task.group) == m_workQueues.end()) {

                std::cout << "TaskExecutor: Work queue " << task.group << " not found!" << std::endl;

                return;
            }

            auto& queue = m_workQueues.at(task.group);

            if(runOnSelf) {

                unsigned int tId = m_threadIdToIndex.at(std::this_thread::get_id());

                queue.second.at(tId)->feed(task);
                queue.second.at(tId)->trigger();

            } else {

                if(queue.second.size() == 1) {

                    queue.second.at(0)->feed(task);
                    queue.second.at(0)->trigger();

                } else {

                    bool taskHandled = false;

                    for(int i = 0; i < queue.second.size(); i++) {

                        if(queue.second.at(queue.first)->isAvailable()) {
 
                            queue.second.at(queue.first)->feed(task);
                            queue.second.at(queue.first)->trigger();

                            taskHandled = true;
                        }

                        if(taskHandled)
                            break;

                    }

                    if(!taskHandled) {

                        queue.second.at(queue.first)->feed(task);
                        queue.second.at(queue.first)->trigger();

                        queue.first = (queue.first + 1) % queue.second.size();                                                        
                    }
                }
            }
        }
    }

    void createQueue(std::string name, unsigned int numWorkers) {

        if(numWorkers == 0) {

            std::cout << "TaskExecutor: work queue must have at least 1 worker thread!" << std::endl;

            return;
        }

        auto queue_itr = m_workQueues.find(name);

        if(queue_itr == m_workQueues.end()) {

            m_workQueues.insert({name, std::pair<unsigned int, std::vector<Worker*>>()});
        }

        auto& workQueue = m_workQueues.at(name);

        workQueue.first = 0;

        for(int i = 0; i < numWorkers; i++) {

            Worker* worker = new Worker(workQueue.second.size(), nullptr);

            worker->start();

            workQueue.second.push_back(worker);

            std::cout << "CreateQueue: Thread id: " << worker->getThreadId() << std::endl;

            m_threadIdToIndex.insert({worker->getThreadId(), workQueue.second.size() - 1});
        }
    }

    void addTasks(std::vector<Task>& tasks) {

        for(int i = 0; i < tasks.size(); i++) {

            if(m_isActive) {
                addTask(tasks[i]);
            }
        }
    }

};