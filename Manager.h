#pragma once

#include "IManager.h"
#include "Dispatcher.h"
#include "Task.h"
#include "TaskQueue.h"
#include <thread>
#include <vector>
#include <functional>


template<typename T>
class Manager : public IManager{
private:
    IDispatcher&  m_dispatcher;

    std::vector<Resource> m_resourceQueue;


    std::string           m_name;
    bool                  m_isRunning;

protected:

    TaskQueue&            m_workQueue;
    std::string			  m_worker;



public:

    Manager(IDispatcher& dispatcher, TaskQueue& taskQueue, std::string name) : 
            m_dispatcher(dispatcher), 
            m_workQueue(taskQueue) 
    {

        m_name = name;
        m_isRunning = true;

        m_worker = name + "_main_thread";

        m_dispatcher.registerManager(m_name, this);
    }

    void addResources(std::vector<Resource>& resources) {

        m_resourceQueue.insert(m_resourceQueue.end(), resources.begin(), resources.end());

        if(isRunning()) {

            m_workQueue.addTask(Task(
                m_worker,
                std::bind(&Manager<T>::execute, this)
            ));
        }
    }

    virtual void execute() = 0;

    void addResource(Resource& resource) {

        m_resourceQueue.push_back(std::move(resource));

        m_workQueue.addTask(Task(
            m_worker,
            std::bind(&Manager<T>::execute, this)
        ));
    }

    std::string getName() {
        return m_name;
    }

    void getResources(int num, std::vector<Resource>& resources) {

        for(int i = 0; i < num && m_resourceQueue.size() > 0; i++) {

            resources.push_back(std::move(m_resourceQueue.back()));
            m_resourceQueue.pop_back();
        }
    }

    int getNumPendingResources() {

        return m_resourceQueue.size();
    }

    void putResources(std::vector<Resource>& resources) {

        m_dispatcher.dispatch(resources);
    }

    bool isRunning() {
        return m_isRunning;
    }

    void stop() {

        m_isRunning = false;
    }

    void start() {

        m_isRunning = true;

        m_workQueue.addTask(Task(
            m_worker,
            std::bind(&Manager<T>::execute, this)
        ));

    }
};