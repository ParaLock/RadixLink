
#pragma once 

#include <string>
#include <functional>
#include <mutex>
#include <map>
#include <utility>


#include "RingBuffer.h"

class StateRegistry {
private:

    struct State {
        
        void*                 data;

        State() {}

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

    struct StateInfo {


        std::function<void(void*&)> deleteState;
        std::function<void(void*&)> createState;

        template<class T>
        void init() {

            createState = [](void*& data) {

                data = new T;
            };

            deleteState = [](void*& data) {

                delete (T*)data;
            };
        }

        void create(void*& data) {

            createState(data);
        }

        void _delete(void*& data) {

            deleteState(data);
        }

    };

    std::map<std::string, std::pair<StateInfo, RingBuffer<State>>> m_registeredStates;
    std::mutex                               m_updateLock;
    int                                      m_maxDepth;

    int                                      m_currState;

public:

    StateRegistry(int maxDepth) {

        m_maxDepth = maxDepth;
        m_currState = 0;
    }

    template<typename T>
    void addState(std::string name, T defValue) {

        StateInfo info;
        info.template init<T>();

		//RingBuffer<State> newRing(m_maxDepth);

        m_registeredStates.insert({name, std::make_pair(info, RingBuffer<State>(m_maxDepth))});

		auto& collection = m_registeredStates.at(name);
		auto& ring = collection.second;

		State stateVal;
		info.create(stateVal.data);
		stateVal.copy<T>(defValue);


		ring.initAll([&info](State& slot) {

			info.create(slot.data);

		}, stateVal);
	}

    template<typename T>
    bool updateState(std::string name, T value) {

        m_updateLock.lock();
        
        auto& stateCollection = m_registeredStates.at(name);

        auto& info = stateCollection.first;
        auto& storage = stateCollection.second;

        bool isFull = false;

        storage.template putInner<T>(value, isFull);

        if(isFull) {
        }

        m_updateLock.unlock();

        return true;
    }

    template<typename T>
    T getState(std::string name) {

        m_updateLock.lock();

        auto& group = m_registeredStates.at(name);

        auto& buff = group.second;
		State state;
		T val;

		state = buff.get();
		state.getVal(val);

		m_updateLock.unlock();

		return val;
	}

};