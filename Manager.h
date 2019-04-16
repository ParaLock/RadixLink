#pragma once

#include "IManager.h"
#include "Dispatcher.h"
#include "Task.h"
#include "StateRegistry.h"
#include "TaskQueue.h"
#include <thread>
#include <vector>
#include <functional>


const static unsigned int HIGH_WATER_MARK = 150;

template<typename T>
class Manager : public IManager{
private:
    IDispatcher&  m_dispatcher;

    std::map<std::string, std::vector<Resource>> m_resourceGroups;

    std::string           m_name;
    bool                  m_isRunning;

    void createGroupIfNotPresent(std::string groupName) {

        auto itr = m_resourceGroups.find(groupName);

        if(itr == m_resourceGroups.end()) {

            m_resourceGroups.insert({groupName, std::vector<Resource>()});

        }
    }

protected:

    TaskQueue&            m_workQueue;
    std::string			  m_worker;
    StateRegistry&        m_stateReg;

	bool				  m_isPolling;

public:

    Manager(IDispatcher& dispatcher, TaskQueue& taskQueue, StateRegistry& reg, std::string name, bool isPolling) : 
            m_dispatcher(dispatcher), 
            m_workQueue(taskQueue),
            m_stateReg(reg)
    {

        m_name = name;
        m_isRunning = true;

		m_isPolling = isPolling;

        m_worker = name + "_main_thread";

        m_dispatcher.registerManager(m_name, this);
    }

    void addResources(std::vector<Resource>& resources, std::string group) {

        std::string groupName = m_name + "-" + group;

        createGroupIfNotPresent(groupName);

        std::vector<Resource>& resourceQueue = m_resourceGroups.at(group);

        if(resourceQueue.size() >= HIGH_WATER_MARK) {

            std::cout << "Manager: High water mark reached! size: " << resourceQueue.size() << std::endl; 
        }


        resourceQueue.insert(resourceQueue.end(), resources.begin(), resources.end());

        if(isRunning() && !m_isPolling) {

            m_workQueue.addTask(Task(
                m_worker,
                std::bind(&Manager<T>::execute, this)
            ));
        }
    }

    virtual void execute() = 0;

    void addResource(Resource& resource, std::string group) {

        std::string groupName = m_name + "-" + group;

        createGroupIfNotPresent(groupName);

        auto& resVec = m_resourceGroups.at(groupName);

        resVec.push_back(std::move(resource));

        if(resVec.size() >= HIGH_WATER_MARK) {

            std::cout << "Manager: High water mark reached! size: " << resVec.size() << std::endl; 
        }

		if (isRunning() && !m_isPolling) {

			m_workQueue.addTask(Task(
				m_worker,
				std::bind(&Manager<T>::execute, this)
			));
		}
    }

    std::string getName() {
        return m_name;
    }

    void getResources(int num, std::vector<Resource>& resources, std::string group) {

        std::string groupName = m_name + "-" + group;

        createGroupIfNotPresent(groupName);

        auto& resVec = m_resourceGroups.at(groupName);

        for(int i = 0; i < num && resVec.size() > 0; i++) {

            resources.push_back(std::move(resVec.back()));
            resVec.pop_back();
        }
    }

    int getNumPendingResources(std::string group) {

        std::string groupName = m_name + "-" + group;

        createGroupIfNotPresent(groupName);

        auto& resVec = m_resourceGroups.at(groupName);

        return resVec.size();
    }

    void putResources(std::vector<Resource>& resources, std::string group) {

        m_dispatcher.dispatch(resources, group);
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