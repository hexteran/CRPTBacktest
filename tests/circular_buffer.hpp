#pragma once

#include <gtest/gtest.h>

#include "../src/utils/circular_buffer.hpp"

using namespace CRPT::Utils;

TEST(Utils, CircularBufferTest_PushPopBack)
{
    CircularBuffer<int, 5> buffer;
    buffer.PushBack(1);
    EXPECT_EQ(buffer.Front(), buffer.Back());
    EXPECT_EQ(buffer.Front(), 1);
    buffer.PushBack(2);
    buffer.PushBack(3);
    buffer.PushBack(4);
    EXPECT_TRUE(buffer.PushBack(5));
    EXPECT_FALSE(buffer.PushBack(6));
    EXPECT_EQ(buffer.Back(), 5);
    EXPECT_TRUE(buffer.PopBack());
    EXPECT_EQ(buffer.Back(), 4);
    EXPECT_TRUE(buffer.PopBack());
    EXPECT_TRUE(buffer.PopBack());
    EXPECT_TRUE(buffer.PopBack());
    EXPECT_EQ(buffer.Back(), 1);
    EXPECT_TRUE(buffer.PopBack());
    EXPECT_FALSE(buffer.PopBack());
}

TEST(Utils, CircularBufferTest_PushPopFront)
{
    CircularBuffer<int, 5> buffer;
    buffer.PushFront(1);
    EXPECT_EQ(buffer.Front(), buffer.Back());
    EXPECT_EQ(buffer.Front(), 1);
    buffer.PushFront(2);
    buffer.PushFront(3);
    buffer.PushFront(4);
    EXPECT_TRUE(buffer.PushFront(5));
    EXPECT_FALSE(buffer.PushFront(6));
    EXPECT_EQ(buffer.Back(), 1);
    EXPECT_EQ(buffer.Front(), 5);
    EXPECT_TRUE(buffer.PopFront());
    EXPECT_EQ(buffer.Front(), 4);
    EXPECT_TRUE(buffer.PopFront());
    EXPECT_TRUE(buffer.PopFront());
    EXPECT_TRUE(buffer.PopFront());
    EXPECT_EQ(buffer.Front(), 1);
    EXPECT_TRUE(buffer.PopFront());
    EXPECT_FALSE(buffer.PopFront());
}

TEST(Utils, CircularBufferTest_PushPopBackFront)
{
    CircularBuffer<int, 5> buffer;
    buffer.PushBack(1);
    EXPECT_EQ(buffer.Front(), buffer.Back());
    EXPECT_EQ(buffer.Front(), 1);
    buffer.PushBack(2);
    buffer.PushBack(3);
    buffer.PushBack(4);
    EXPECT_TRUE(buffer.PushBack(5));
    EXPECT_FALSE(buffer.PushBack(6));
    EXPECT_EQ(buffer.Back(), 5);
    EXPECT_TRUE(buffer.PopBack());
    EXPECT_EQ(buffer.Back(), 4);
    EXPECT_TRUE(buffer.PopBack());
    EXPECT_TRUE(buffer.PopBack());
    EXPECT_TRUE(buffer.PopBack());
    EXPECT_EQ(buffer.Back(), 1);
    EXPECT_TRUE(buffer.PopBack());
    EXPECT_FALSE(buffer.PopBack());

    buffer.PushFront(1);
    EXPECT_EQ(buffer.Front(), buffer.Back());
    EXPECT_EQ(buffer.Front(), 1);
    buffer.PushFront(2);
    buffer.PushFront(3);
    buffer.PushFront(4);
    EXPECT_TRUE(buffer.PushFront(5));
    EXPECT_FALSE(buffer.PushFront(6));
    EXPECT_EQ(buffer.Back(), 1);
    EXPECT_EQ(buffer.Front(), 5);
    EXPECT_TRUE(buffer.PopFront());
    EXPECT_EQ(buffer.Front(), 4);
    EXPECT_TRUE(buffer.PopFront());
    EXPECT_TRUE(buffer.PopFront());
    EXPECT_TRUE(buffer.PopFront());
    EXPECT_EQ(buffer.Front(), 1);
    EXPECT_TRUE(buffer.PopFront());
    EXPECT_FALSE(buffer.PopFront());
}