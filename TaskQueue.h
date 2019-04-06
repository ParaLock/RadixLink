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

        if(mutex)
            delete mutex;
        if(workAvailable)
            delete workAvailable;

        if(workerThread) {

            isRunning = false;
            workerThread->join();

            delete workerThread;

        }

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

public:

    void addTask(Task task) {

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

    void addTasks(std::vector<Task>& tasks) {

        for(int i = 0; i < tasks.size(); i++) {

            addTask(tasks[i]);
        }
    }

};