
#pragma once 

#include <string>
#include <functional>
#include <mutex>

class StateRegistry {
private:

    struct State {

        void*                 data;

        std::function<void()> deleteState;
        std::function<void()> createState;

        template<typename T>
        void init() {

            createState = [this]() {

                data = new T;
            };

            deleteState = [this]() {

                delete (T*)data;
            };
        }

        void create() {

            createState();
        }

        void _delete() {

            deleteState();
        }

        template<typename T>
        bool copy(T& state) {

            *(T*)data = state; 

            return true;
        }

        template<typename T>
        void getVal(T& val) {

            val = *(T*)data;
        }
        
    };

    std::map<std::string, State> m_registeredStates;
    std::mutex                   m_updateLock;

public:

    template<typename T>
    void addState(std::string name) {

        State newState;
        newState.init<T>();

        m_registeredStates.insert({name, newState});

        State& state = m_registeredStates.at(name);

        state.create();

    }

    template<typename T>
    bool updateState(std::string name, T value) {

        m_updateLock.lock();
        
        State& state = m_registeredStates.at(name);

        if(!state.copy<T>(value)) {

            std::cout << "StateRegistry: Bad state copy! state: " << name << std::endl; 

            m_updateLock.unlock();

            return false;
        }

        m_updateLock.unlock();

        return true;
    }

    template<typename T>
    T getState(std::string name) {

        m_updateLock.lock();

        State& state = m_registeredStates.at(name);

        T val;
        state.getVal(val);

        m_updateLock.unlock();

        return val;
    }

};