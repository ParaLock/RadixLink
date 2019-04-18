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

#include "Task.h"

struct Worker {

    std::thread* workerThread;
    bool        isRunning;
    std::deque<Task> work;

    std::mutex*              mutex;
    std::condition_variable* workAvailable;
    bool                    isWorkAvailable;

    Worker() {

        mutex = nullptr;
        workAvailable = nullptr;

        isRunning = true;

        isWorkAvailable = false;

        workerThread = nullptr;

    }

    ~Worker() {



    }

    void stop() {

        if(workerThread && mutex && workAvailable) {

            isRunning = false;
            isWorkAvailable = true;
            workAvailable->notify_one();

            workerThread->join();

            delete workerThread;

        }

        if(mutex)
            delete mutex;
        if(workAvailable)
            delete workAvailable;
    }

    void start() {

        mutex           = new std::mutex;
        workAvailable   = new std::condition_variable;
        workerThread    = new std::thread(&Worker::exec, this);
    }

    bool isAvailable() {
        return isWorkAvailable;
    }

    void exec() {

        while(isRunning) {
            
            std::unique_lock<std::mutex> lk(*mutex);
            workAvailable->wait(lk, [this]() { return this->isWorkAvailable; });

            size_t wSize = work.size();

            while(wSize > 0) {
                
                Task& task = work.front();

                task.func();

                if(task.completionCallback) {

                    task.completionCallback();
                }

				work.pop_front();

                wSize--;

            }

            isWorkAvailable = work.size() > 0;

            lk.unlock();
        }
    }
};


class TaskQueue {
private:
    
    std::map<std::string, Worker> m_taskGroups;

    bool m_isActive;

public:

    TaskQueue() {

        m_isActive = true;
    }

    ~TaskQueue() {

        
    }

    void stop() {

        m_isActive = false;

        for (auto& worker : m_taskGroups) {

            std::cout << "Thread Model: Stopping OR Moving worker thread." << std::endl;

            worker.second.stop();

            std::cout << "Thread Model: Worker Thread Joined." << std::endl;
        }
    }

    void addTask(Task task) {

        if(m_isActive) {

            auto itr = m_taskGroups.find(task.group);

            if(itr == m_taskGroups.end()) {

                m_taskGroups.insert({task.group, Worker()});

                Worker& temp = m_taskGroups.at(task.group);
                temp.start();
            }

            auto& group = m_taskGroups.at(task.group);

        // std::lock_guard<std::mutex> lk(*group.mutex);

            group.work.push_back(task);

            group.isWorkAvailable = true;
            group.workAvailable->notify_one();
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