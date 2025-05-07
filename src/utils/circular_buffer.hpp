#pragma once

#include "../definitions.h"

#include <iostream>

constexpr uint64_t CT_MAX_BUFFER_SIZE = 10000;

template<class T, uint64_t n>
class CircularBuffer
{
    using BufferType = std::conditional_t<(n > CT_MAX_BUFFER_SIZE),
                                          std::vector<T>,
                                          std::array<T, n>>;

public:
    CircularBuffer(CircularBuffer&&) = delete;
    CircularBuffer(const CircularBuffer&) = delete;

    CircularBuffer()
    {
        if constexpr (n > CT_MAX_BUFFER_SIZE) 
        {
            m_buffer.resize(n);
        }
    }

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

    inline T& Front()
    {
        return m_buffer[m_begin];
    }

    inline T& Back()
    {
        return m_buffer[m_end];
    }

    bool Empty() const
    {
        return m_size == 0;
    }
private:
    BufferType m_buffer;
    int m_begin{0}, m_end{-1};
    int m_size{0};
};