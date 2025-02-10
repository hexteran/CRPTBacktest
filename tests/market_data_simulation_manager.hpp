#pragma once

#include <gtest/gtest.h>

#include "../src/core/market_data_simulation_manager.hpp"

using BufferPtr = char*;

TEST(MDRow, AllOperations)
{
    std::vector<MDCustomUpdate> updates1(5, MDCustomUpdate());
    for (int i = 0; i < updates1.size(); ++i)
    {
        updates1[i].EventTimestamp = i*2 + 1;
        updates1[i].Payload = i;
    }

    MDRow row1(updates1);
    for(int i = 0; i < row1.size(); ++i)
    {
        auto r = row1[i];
        EXPECT_EQ(r->EventTimestamp, i*2 + 1);
        EXPECT_EQ(r->Type, MarketDataType::Custom);
        EXPECT_EQ(MDCustomUpdatePtr(r)->Payload, double(i));
    }
}

TEST(MarketDataSimulationManagerTests, IteratorInc)
{
    std::vector<MDCustomUpdate> updates1(5, MDCustomUpdate());
    std::vector<MDCustomUpdate> updates2(5, MDCustomUpdate());
    for (int i = 0; i < updates1.size(); ++i)
        updates1[i].EventTimestamp = i*2 + 1;
    for (int i = 0; i < updates2.size(); ++i)
        updates2[i].EventTimestamp = i*2;
    MarketDataSimulationManager manager(std::vector<MDRow>{updates1, updates2});
    auto iter = manager.begin();
    auto end = manager.end();
    EXPECT_FALSE(iter == end);
    for(int counter = 0; iter != manager.end(); ++iter)
    {
        EXPECT_EQ(iter->EventTimestamp, counter);
        ++counter;
        //std::cout << iter->EventTimestamp << '\n';
    }
}

TEST(MarketDataSimulationManagerTests, IteratorPlus)
{
    std::vector<MDCustomUpdate> updates1(5, MDCustomUpdate());
    std::vector<MDCustomUpdate> updates2(5, MDCustomUpdate());
    std::vector<MDTrade> updates3(5, MDTrade());
    for (int i = 0; i < updates1.size(); ++i)
        updates1[i].EventTimestamp = i*3;
    for (int i = 0; i < updates2.size(); ++i)
        updates2[i].EventTimestamp = i*3 + 1;
    for (int i = 0; i < updates3.size(); ++i)
    {
        updates3[i].EventTimestamp = i*3 + 2;
        updates3[i].Price = 2;
    }
    MarketDataSimulationManager manager(std::vector<MDRow>{updates1, updates2, updates3});
    auto iter = manager.begin();
    auto end = manager.end();
    EXPECT_FALSE(iter == end);
    for(int counter = 0; iter != manager.end(); ++iter)
    {
        EXPECT_EQ(iter->EventTimestamp, counter);
        ++counter;
        if(counter == 5)
        {
            auto iter_1 = iter+1;
            auto iter_2 = iter+2;
            EXPECT_EQ(iter_1->EventTimestamp, counter + 1);
            EXPECT_EQ(iter_2->EventTimestamp, counter + 2);
            ++iter_2;
            EXPECT_EQ(iter_2->EventTimestamp, counter + 3);
        }
        if (iter->Type == MarketDataType::Trade)
        {
            EXPECT_EQ(MDTradePtr(*iter)->Price, 2.);
        }
    }
}