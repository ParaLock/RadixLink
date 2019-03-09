#pragma once

#include "IManager.h"
#include "Dispatcher.h"
#include <thread>
#include <vector>
#include <functional>

template<typename T>
class Manager : public IManager{
private:
    std::thread m_executionThread;
    IDispatcher&  m_dispatcher;

    std::vector<Resource> m_resourceQueue;

    std::string           m_name;
public:

    Manager(IDispatcher& dispatcher, std::string name) : m_dispatcher(dispatcher) {

        m_name = name;

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

    void start() {

        m_executionThread = std::thread([this]() { this->execute();});
    }

    void stop() {

        m_executionThread.join();
    }
};