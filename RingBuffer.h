#pragma once
#include <cstdio>

#include <memory>
#include <memory>
#include <mutex>

template <typename T>
class RingBuffer {
private:
    size_t m_size = 0;
	size_t m_head = 0;
	size_t m_tail = 0;
	bool m_full = 0;
	T m_defValue;

	std::vector<T> m_items;

    void update() {

		if(m_full)
		{
			m_tail = (m_tail + 1) % m_size;
		}

		m_head = (m_head + 1) % m_size;

		m_full = m_head == m_tail;
        
    }

public:
	RingBuffer(size_t size)
    {
		for (int i = 0; i < size; i++) {

			m_items.push_back(T());
		}

        m_size = size;
	}
    
    RingBuffer(const RingBuffer& other) {

        m_items = other.m_items;
        m_full = other.m_full;
        m_head = other.m_head;
        m_tail = other.m_tail;
        m_size = other.m_size;
		m_defValue = other.m_defValue;
    }

    RingBuffer& initAll(std::function<void(T&)> callback, T& defValue) {

        for(int i = 0; i < m_size; i++) {

            callback(m_items[i]);
        }

		m_defValue = defValue;

        return *this;
    }

    template<typename T_INNER>
    void putInner(T_INNER val, bool& isFull) {

        isFull = m_full;

        T& item = m_items[m_head];

        item.copy(val);

        update();
    }

	void put(T item) {

		m_items[m_head] = item;
        
        update();
	}

	bool get(T& val) {

		if(empty())
		{
			val = m_defValue;

			return false;
		}

		val = m_items[m_tail];
		m_full = false;
		m_tail = (m_tail + 1) % m_size;

		return true;
	}

	void reset() {

		m_head = m_tail;
		m_full = false;
	}

	bool empty() {

		return (!m_full && (m_head == m_tail));
	}

	bool full() {

		return m_full;
	}

	size_t capacity() {

		return m_size;
	}

	size_t size() {

		size_t size = m_size;

		if(!m_full) {

			if(m_head >= m_tail) {

				size = m_head - m_tail;
			}
			else {

				size = m_size + m_head - m_tail;
			}
		}

		return size;
	}

};
