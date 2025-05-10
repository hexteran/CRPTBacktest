#pragma once

#include <gtest/gtest.h>

#include "../src/core/order_execution_manager.hpp"

using namespace CRPT::Core;

TEST(OrderExecutionManagerTests, EmptyMatch)
{
    OrderExecutionManager manager;
    auto resultBuy = manager.MatchWithPrice(100.0, Side::Buy);
    EXPECT_TRUE(resultBuy.empty());
    
    auto resultSell = manager.MatchWithPrice(100.0, Side::Sell);
    EXPECT_TRUE(resultSell.empty());
}

TEST(OrderExecutionManagerTests, MatchBuyOrdersWithPrice)
{
    Instrument* instrument = new Instrument{"INST1", "Venue1"};
    OrderExecutionManager manager;
    
    // Create a buy limit order at 100 (should match when threshold is 95)
    OrderPtr order1 = new Order();
    order1->Id = 1;
    order1->ClOrdId = "order1";
    order1->Type = OrderType::Limit;
    order1->OrderSide = Side::Buy;
    order1->Price = 100.0;
    order1->Qty = 10;
    order1->Instrument = instrument->InstrumentId;
    
    // Create a buy limit order at 90 (should NOT match when threshold is 95)
    OrderPtr order2 = new Order();
    order2->Id = 2;
    order2->ClOrdId = "order2";
    order2->Type = OrderType::Limit;
    order2->OrderSide = Side::Buy;
    order2->Price = 90.0;
    order2->Qty = 10;
    order2->Instrument = instrument->InstrumentId;
    
    // Create a buy market order (always matches)
    OrderPtr order3 = new Order();
    order3->Id = 3;
    order3->ClOrdId = "order3";
    order3->Type = OrderType::Market;
    order3->OrderSide = Side::Buy;
    order3->Price = 0.0; // Price is irrelevant for market orders.
    order3->Qty = 10;
    order3->Instrument = instrument->InstrumentId;
    
    manager.AddNewOrder(order1);
    manager.AddNewOrder(order2);
    manager.AddNewOrder(order3);
    
    // Match with a threshold of 95.
    auto executedOrders = manager.MatchWithPrice(95.0, Side::Sell);
    // We expect order1 (price 100) and order3 (market) to be executed.
    EXPECT_EQ(executedOrders.size(), 2u);
    bool foundOrder1 = false, foundOrder3 = false;
    for(auto order : executedOrders)
    {
        if(order->Id == 1) { foundOrder1 = true; }
        if(order->Id == 3) { foundOrder3 = true; }
    }
    EXPECT_TRUE(foundOrder1);
    EXPECT_TRUE(foundOrder3);
    
    // The remaining order (order2 at 90) should not have matched at 95.
    // Now match with a lower threshold.
    auto executedOrders2 = manager.MatchWithPrice(85.0, Side::Sell);
    EXPECT_EQ(executedOrders2.size(), 1u);
    EXPECT_EQ(executedOrders2[0]->Id, 2);
    
    // Clean up.
    delete order1;
    delete order2;
    delete order3;
    delete instrument;
}

// Test that sell orders are matched only if their price is low enough,
// or if they are market orders.
TEST(OrderExecutionManagerTests, MatchSellOrdersWithPrice)
{
    Instrument* instrument = new Instrument{"INST2", "Venue2"};
    OrderExecutionManager manager;
    
    // Create a sell limit order at 100 (should match when threshold is 105)
    OrderPtr order1 = new Order();
    order1->Id = 1;
    order1->ClOrdId = "order1";
    order1->Type = OrderType::Limit;
    order1->OrderSide = Side::Sell;
    order1->Price = 100.0;
    order1->Qty = 10;
    order1->Instrument = instrument->InstrumentId;
    
    // Create a sell limit order at 110 (should NOT match when threshold is 105)
    OrderPtr order2 = new Order();
    order2->Id = 2;
    order2->ClOrdId = "order2";
    order2->Type = OrderType::Limit;
    order2->OrderSide = Side::Sell;
    order2->Price = 110.0;
    order2->Qty = 10;
    order2->Instrument = instrument->InstrumentId;
    
    // Create a sell market order (always matches)
    OrderPtr order3 = new Order();
    order3->Id = 3;
    order3->ClOrdId = "order3";
    order3->Type = OrderType::Market;
    order3->OrderSide = Side::Sell;
    order3->Price = 0.0; // Price is irrelevant.
    order3->Qty = 10;
    order3->Instrument = instrument->InstrumentId;
    
    manager.AddNewOrder(order1);
    manager.AddNewOrder(order2);
    manager.AddNewOrder(order3);
    
    // Match with a threshold of 105.
    auto executedOrders = manager.MatchWithPrice(105.0, Side::Buy);
    // We expect order1 (price 100) and order3 (market) to be executed.
    EXPECT_EQ(executedOrders.size(), 2u);
    bool foundOrder1 = false, foundOrder3 = false;
    for(auto order : executedOrders)
    {
        if(order->Id == 1) { foundOrder1 = true; }
        if(order->Id == 3) { foundOrder3 = true; }
    }
    EXPECT_TRUE(foundOrder1);
    EXPECT_TRUE(foundOrder3);
    
    // Now match the remaining order (order2) with a higher threshold.
    auto executedOrders2 = manager.MatchWithPrice(115.0, Side::Buy);
    EXPECT_EQ(executedOrders2.size(), 1u);
    EXPECT_EQ(executedOrders2[0]->Id, 2);
    
    // Clean up.
    delete order1;
    delete order2;
    delete order3;
    delete instrument;
}

// Test that market orders always match regardless of the threshold.
TEST(OrderExecutionManagerTests, MarketOrdersAlwaysMatch)
{
    Instrument* instrument = new Instrument{"INST3", "Venue3"};
    OrderExecutionManager manager;
    
    // Create a buy market order.
    OrderPtr buyMarket = new Order();
    buyMarket->Id = 1;
    buyMarket->ClOrdId = "buyMarket";
    buyMarket->Type = OrderType::Market;
    buyMarket->OrderSide = Side::Buy;
    buyMarket->Price = 0.0;
    buyMarket->Qty = 10;
    buyMarket->Instrument = instrument->InstrumentId;
    
    // Create a sell market order.
    OrderPtr sellMarket = new Order();
    sellMarket->Id = 2;
    sellMarket->ClOrdId = "sellMarket";
    sellMarket->Type = OrderType::Market;
    sellMarket->OrderSide = Side::Sell;
    sellMarket->Price = 0.0;
    sellMarket->Qty = 10;
    sellMarket->Instrument = instrument->InstrumentId;
    
    manager.AddNewOrder(buyMarket);
    manager.AddNewOrder(sellMarket);
    
    // For buy orders, even a very high threshold should match a market order.
    auto executedBuy = manager.MatchWithPrice(1000.0, Side::Sell);
    EXPECT_EQ(executedBuy.size(), 1u);
    EXPECT_EQ(executedBuy[0]->Id, 1);
    
    // For sell orders, even a very low threshold should match a market order.
    auto executedSell = manager.MatchWithPrice(0.0, Side::Buy);
    EXPECT_EQ(executedSell.size(), 1u);
    EXPECT_EQ(executedSell[0]->Id, 2);
    
    // Clean up.
    delete buyMarket;
    delete sellMarket;
    delete instrument;
}