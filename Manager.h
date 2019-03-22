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


public:

    Manager(IDispatcher& dispatcher, TaskQueue& taskQueue, std::string name) : 
            m_dispatcher(dispatcher), 
            m_workQueue(taskQueue) 
    {

        m_name = name;
        m_isRunning = true;

        m_dispatcher.registerManager(m_name, this);
    }

    void addResources(std::vector<Resource>& resources) {

        m_resourceQueue.insert(m_resourceQueue.end(), resources.begin(), resources.end());
    }

    void addResource(Resource& resource) {

        m_resourceQueue.push_back(std::move(resource));
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

};