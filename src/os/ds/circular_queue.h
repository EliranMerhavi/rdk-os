#pragma once
#include "assert.h"

template<typename t, size_t size>
class circular_queue_t
{
public:
    circular_queue_t() : m_rear(-1), m_front(-1), m_data()
    {
    }

    bool push(const t& element)
    {
        if (full()) {
            return false;
        }
        
        if (m_front == -1)
            m_front = m_rear = 0;
        else 
            m_rear = (m_rear + 1) % size;

        m_data[m_rear] = element;
        return true;
    }

    void pop()
    {
        assert(!empty());

        if (m_front == m_rear) {
            m_front = m_rear = -1;
        }
        else {
            m_front = (m_front + 1) % size;
        }
    }

    void pop_back()
    {
        assert(!empty());
        if (m_front == m_rear) {
            m_front = m_rear = -1;
        } 
        else {
            m_rear = (m_rear - 1) % size;
        }
    }

    t& front()
    {
        assert(!empty());
        return m_data[m_front];
    }

    const t& front() const 
    {
        assert(!empty());
        return m_data[m_front];
    }

    bool empty() const
    {
        return m_front == -1;
    }

    bool full() const
    {
        return ((!m_front && m_rear == size - 1) || ((int)((m_rear + 1) % size) == m_front));
    }
private:
    int m_rear, m_front;
    t m_data[size];
};
