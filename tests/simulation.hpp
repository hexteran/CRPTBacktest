#pragma once

#include <gtest/gtest.h>

#include "../src/core/simulation.hpp"

static std::vector<OrderPtr> g_executedOrders;
static std::vector<OrderPtr> g_canceledOrders;
static std::vector<OrderPtr> g_replacedOrders;
static std::vector<OrderPtr> g_newOrders;
static std::vector<MDTradePtr> g_mdTrades;

void ExecutedOrderCallback(OrderPtr order) {
    g_executedOrders.push_back(order);
}

void CanceledOrderCallback(OrderPtr order) {
    g_canceledOrders.push_back(order);
}

void ReplacedOrderCallback(OrderPtr order) {
    g_replacedOrders.push_back(order);
}

void NewOrderCallback(OrderPtr order) {
    g_newOrders.push_back(order);
}

void MDTradeCallback(MDTradePtr trade) {
    g_mdTrades.push_back(trade);
}

class FakeCSVMarketDataTradesManager {
public:
    using iterator = std::vector<MDTrade>::iterator;
    using const_iterator = std::vector<MDTrade>::const_iterator;
    std::vector<MDTrade> trades;
    
    iterator begin() { return trades.begin(); }
    iterator end()   { return trades.end(); }
};

// Test that a new order is processed and eventually executed when market data arrives.
TEST(SimulationTests, ExecuteOrderTest) {
    // Reset global recording vectors.
    g_executedOrders.clear();
    g_canceledOrders.clear();
    g_newOrders.clear();
    g_mdTrades.clear();

    // Set up a fake market data manager with two MDTrade events.
    CSVMarketDataTradesManager dataCollection({"../../data/simulation_test_trades_0.csv"});
    auto row = dataCollection.GetTrades();
    MDRow mdRow(row);
    MarketDataSimulationManager marketDataManager({MDRow{row}});

    // Create a Simulation with execution latency 10 and market data latency 5.
    Simulation<10> sim(marketDataManager, 10, 5,
        ExecutedOrderCallback, CanceledOrderCallback, ReplacedOrderCallback, NewOrderCallback, MDTradeCallback);

    // Create a buy limit order (price 105; should match when market trade price is 100).
    OrderPtr order = new Order();
    order->Id = 1;
    order->OrderSide = Side::Buy;
    order->Type = OrderType::Limit;
    order->Price = 105;
    order->Qty = 10;

    // Submit the order.
    sim.OnNewOrder(order);

    // Run the simulation.
    sim.Run();

    // Verify that the executed order callback was invoked.
    ASSERT_EQ(g_executedOrders.size(), 1u);
    EXPECT_EQ(g_executedOrders[0]->Id, 1);
    EXPECT_EQ(g_executedOrders[0]->State, OrderState::Filled);

    // Verify that the MDTrade callback was invoked (the first trade’s local time is reached later).
    ASSERT_EQ(g_mdTrades.size(), 1u);
    // (For example, the first trade’s LocalTimestamp equals 25+5 = 30,
    // and is processed when the simulation’s current timestamp is 35.)
    EXPECT_GE(g_mdTrades[0]->LocalTimestamp, 30);

    // Clean up.
    delete order;
}

// Test that a buy limit order is executed only when a later MDTrade event
// has a price low enough (i.e. order->Price >= mdTrade Price).
TEST(SimulationTests, LimitBuyOrderExecutionTest) {
    // Clear previous callback records.
    g_executedOrders.clear();
    g_canceledOrders.clear();
    g_newOrders.clear();
    g_mdTrades.clear();

    // Create market data events:
    // First MDTrade event: price 105 (not favorable for a buy order at 100).
    CSVMarketDataTradesManager dataCollection({"../../data/simulation_test_trades_3.csv"});
    auto row = dataCollection.GetTrades();
    MDRow mdRow(row);
    MarketDataSimulationManager marketDataManager({MDRow{row}});
    // Create Simulation with executionLatency=10, marketDataLatency=5.
    Simulation<10> sim(marketDataManager, 10, 5,
        ExecutedOrderCallback, CanceledOrderCallback, ReplacedOrderCallback, NewOrderCallback, MDTradeCallback);

    // Create a buy limit order with limit price 100.
    OrderPtr order = new Order();
    order->Id = 101;
    order->OrderSide = Side::Buy;
    order->Type = OrderType::Limit;
    order->Price = 100;
    order->Qty = 50;

    // Submit the order at time 0.
    sim.OnNewOrder(order);

    // Run the simulation.
    sim.Run();

    // Expect that the order is executed during the second MDTrade event.
    ASSERT_EQ(g_executedOrders.size(), 1u);
    EXPECT_EQ(g_executedOrders[0]->Id, 101);
    EXPECT_EQ(g_executedOrders[0]->State, OrderState::Filled);

    delete order;
}

// Test that a sell limit order is executed only when a later MDTrade event
// has a price high enough (i.e. order->Price <= mdTrade Price).
TEST(SimulationTests, LimitSellOrderExecutionTest) {
    g_executedOrders.clear();
    g_canceledOrders.clear();
    g_newOrders.clear();
    g_mdTrades.clear();

    // Create market data events:
    // First event: price 95 (not favorable for a sell order at 100, since 100 <= 95 is false).
    CSVMarketDataTradesManager dataCollection({"../../data/simulation_test_trades_4.csv"});
    auto row = dataCollection.GetTrades();
    MDRow mdRow(row);
    MarketDataSimulationManager marketDataManager({MDRow{row}});
    Simulation<10> sim(marketDataManager, 10, 5,
        ExecutedOrderCallback, CanceledOrderCallback, ReplacedOrderCallback, NewOrderCallback, MDTradeCallback);

    // Create a sell limit order with limit price 100.
    OrderPtr order = new Order();
    order->Id = 202;
    order->OrderSide = Side::Sell;
    order->Type = OrderType::Limit;
    order->Price = 100;
    order->Qty = 30;

    sim.OnNewOrder(order);
    sim.Run();

    ASSERT_EQ(g_executedOrders.size(), 1u);
    EXPECT_EQ(g_newOrders.size(), 1);
    EXPECT_EQ(g_executedOrders[0]->Price, 100);
    EXPECT_EQ(g_executedOrders[0]->State, OrderState::Filled);

    delete order;
}

// Test that a buy limit order is not executed if no market data event meets its price condition.
TEST(SimulationTests, LimitBuyOrderNotExecutedTest) {
    g_executedOrders.clear();
    g_canceledOrders.clear();
    g_newOrders.clear();
    g_mdTrades.clear();

    // Provide a single MDTrade event with a price that does not favor a buy limit order at 100.
    CSVMarketDataTradesManager dataCollection({"../../data/simulation_test_trades_4.csv"});
    auto row = dataCollection.GetTrades();
    MDRow mdRow(row);
    MarketDataSimulationManager marketDataManager({MDRow{row}});

    Simulation<10> sim(marketDataManager, 10, 5,
        ExecutedOrderCallback, CanceledOrderCallback, ReplacedOrderCallback, NewOrderCallback, MDTradeCallback);

    // Create a buy limit order with price 100.
    OrderPtr order = new Order();
    order->Id = 303;
    order->OrderSide = Side::Sell;
    order->Type = OrderType::Limit;
    order->Price = 110;
    order->Qty = 10;

    sim.OnNewOrder(order);
    sim.Run();

    // Expect that no execution occurred.
    EXPECT_TRUE(g_executedOrders.empty());
    EXPECT_EQ(g_newOrders.size(), 1);

    delete order;
}

// Test that a sell limit order is not executed if no market data event meets its price condition.
TEST(SimulationTests, LimitSellOrderNotExecutedTest) {
    g_executedOrders.clear();
    g_canceledOrders.clear();
    g_newOrders.clear();
    g_mdTrades.clear();

    // Provide a single MDTrade event with a price that does not favor a sell limit order at 100.
    CSVMarketDataTradesManager dataCollection({"../../data/simulation_test_trades_3.csv"});
    auto row = dataCollection.GetTrades();
    MDRow mdRow(row);
    MarketDataSimulationManager marketDataManager({MDRow{row}});

    Simulation<10> sim(marketDataManager, 10, 5,
        ExecutedOrderCallback, CanceledOrderCallback, ReplacedOrderCallback, NewOrderCallback, MDTradeCallback);

    // Create a sell limit order with price 100.
    OrderPtr order = new Order();
    order->Id = 404;
    order->OrderSide = Side::Buy;
    order->Type = OrderType::Limit;
    order->Price = 85;
    order->Qty = 15;

    sim.OnNewOrder(order);
    sim.Run();

    EXPECT_TRUE(g_executedOrders.empty());

    delete order;
}

TEST(SimulationTests, LimitBuyOrderCancelTest) {
    // Clear previous callback records.
    g_executedOrders.clear();
    g_canceledOrders.clear();
    g_newOrders.clear();
    g_mdTrades.clear();

    // Create market data events:
    // First MDTrade event: price 105 (not favorable for a buy order at 100).
    CSVMarketDataTradesManager dataCollection({"../../data/simulation_test_trades_5.csv"});
    auto row = dataCollection.GetTrades();
    MDRow mdRow(row);
    MarketDataSimulationManager marketDataManager({MDRow{row}});
    // Create Simulation with executionLatency=10, marketDataLatency=5.

    Simulation<10> sim(marketDataManager, 10, 5,
        ExecutedOrderCallback, CanceledOrderCallback, ReplacedOrderCallback, NewOrderCallback, MDTradeCallback);

    // Create a buy limit order with limit price 100.
    OrderPtr order = new Order();
    order->Id = 101;
    order->OrderSide = Side::Sell;
    order->Type = OrderType::Limit;
    order->Price = 150;
    order->Qty = 50;

    // Submit the order at time 0.
    sim.OnNewOrder(order);

    // Run the simulation.
    sim.Run();

    // Expect that the order is executed during the second MDTrade event.
    EXPECT_EQ(g_executedOrders.size(), 0);

    delete order;
}