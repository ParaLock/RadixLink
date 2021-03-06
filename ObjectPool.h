
#pragma once

#include <deque>
#include <mutex>


namespace ObjectPool {

    struct Pooled {

        unsigned int pool_index;
    };

    template<typename T>
    class Pool {
    private:
        std::deque<unsigned int> m_freeList;
        std::vector<T>            m_pool;

    public:

        Pool(unsigned int numStartingElements) {


            for(int i = 0; i < numStartingElements; i++) {

                m_pool.    push_back(T());

                auto& item = m_pool.back();

                item.pool_index = m_pool.size() - 1;
                item.reset();

                m_freeList.push_back(m_pool.size() - 1);
            }
        }
    
        ~Pool() {

            //delete m_lock;
        }

        T* getItem() {

            if(m_freeList.size() == 0) {

                m_pool.    push_back(T());
                
                auto& item = m_pool.back();
                item.reset();

                m_freeList.push_back(m_pool.size() - 1);
            }

            auto index = m_freeList.back();
            m_freeList.pop_back();

            T* item = &m_pool.at(index);

            item->pool_index = index;

            return item;
        }

        void freeItem(T* item) {

            item->reset();
            m_freeList.push_back(item->pool_index);
        }
    };
}
