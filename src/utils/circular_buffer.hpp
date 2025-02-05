#pragma once

#include <iostream>

template<class T, int n>
class CircularBuffer
{
public:
    bool PushBack(const T& element)
    {
        if (m_size >= n)
            return false;
        else if (m_size == 0)
        {
            m_begin = 0;
            m_end = -1;
        }

        m_buffer[++m_end] = element;
        if(m_end >= n) m_end = 0;
        ++m_size;

        return true;
    }

    bool PushFront(const T& element)
    {
        if (m_size >= n)
            return false;
        else if (m_size == 0)
        {
            m_begin = 1;
            m_end = 0;
        }

        m_buffer[--m_begin] = element;
        if (m_begin < 0) m_begin = n - 1;
        ++m_size;

        return true;
    }

    bool PopBack()
    {
        if (m_size == 0) return false;

        --m_end;
        if (m_end < 0) m_end = n - 1;
        --m_size;

        return true;
    }
    
    bool PopFront()
    {
        if (m_size == 0) return false;

        ++m_begin;
        if (m_begin >= n) m_begin = 0;
        --m_size;

        return true;
    }

    T& Front()
    {
        return m_buffer[m_begin];
    }

    T& Back()
    {
        return m_buffer[m_end];
    }

    bool Empty()
    {
        return m_size == 0;
    }
private:
    std::array<T, n> m_buffer;
    int m_begin{0}, m_end{-1};
    int m_size{0};
};