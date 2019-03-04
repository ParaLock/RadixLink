#include "IManager.h"
#include <thread>

template<typename T>
class Manager {
private:
    std::thread m_executionThread;
    IDispatcher&  m_dispatcher;

    std::vector<Resource> m_resourceQueue;
public:

    Manager(IDispatcher& dispatcher) : m_dispatcher(dispatcher) {

        m_dispatcher.registerManager(typeid(T).name(), *this);
    }

    void addResources(std::vector<Resource>& resources) {

        m_resourceQueue.insert(m_resourceQueue.end(), resources.begin(), resources.end());

    }

    std::vector<Resource> getResources(int num) {

        std::vector<Resource> temp;

        for(int i = 0; i < num && m_resourceQueue.size() > 0; i++) {

            temp.push_back(m_resourceQueue.back());
            m_resourceQueue.pop_back();
        }

        return temp;
    }

    void putResources(std::vector<Resource>& resources) {

        m_dispatcher.dispatch(resources);
    }

    virtual void execute() = 0;

    void start() {

        m_executionThread = std::thread(execute);
    }

    void stop() {

        m_executionThread.join();
    }
};