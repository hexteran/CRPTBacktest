#pragma once

#include <gtest/gtest.h>

#include "../src/core/simulation.hpp"

using namespace CRPT::Core;
using namespace CRPT::Utils;

static std::vector<OrderPtr> g_executedOrders;
static std::vector<OrderPtr> g_canceledOrders;
static std::vector<OrderPtr> g_replacedOrders;
static std::vector<OrderPtr> g_newOrders;
static std::vector<MDTradePtr> g_mdTrades;
static std::vector<MDL1UpdatePtr> g_mdL1Updates;
static std::vector<MDCustomUpdatePtr> g_mdCustomUpdate;
static std::vector<MDCustomMultipleUpdatePtr> g_mdCustomMultipleUpdate;

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

void MDL1UpdateCallback(MDL1UpdatePtr update) {
    g_mdL1Updates.push_back(update);
}

void MDCustomUpdateCallback(MDCustomUpdatePtr update) {
    g_mdCustomUpdate.push_back(update);
}

void MDCustomMultipleUpdateCallback(MDCustomMultipleUpdatePtr update) {
    g_mdCustomMultipleUpdate.push_back(update);
}

TEST(SimulationTests, ExecuteOrderTest) {
    g_executedOrders.clear();
    g_canceledOrders.clear();
    g_newOrders.clear();
    g_mdTrades.clear();
    g_mdL1Updates.clear();

    // Set up a fake market data manager with two MDTrade events.
    CSVMarketDataTradesManager dataCollection({"../../data/simulation_test_trades_0.csv"});
    auto row = dataCollection.GetTrades();
    MDRow mdRow(row);
    MarketDataSimulationManager marketDataManager({MDRow{row}});

    // Create a Simulation with execution latency 10 and market data latency 5.
    Simulation<10> sim(marketDataManager, 10, 5,
        ExecutedOrderCallback, 
        CanceledOrderCallback,
        ReplacedOrderCallback,
        NewOrderCallback,
        MDTradeCallback,
        MDL1UpdateCallback,
        MDCustomUpdateCallback);

    OrderPtr order = new Order();
    order->Id = 1;
    order->OrderSide = Side::Buy;
    order->Type = OrderType::Limit;
    order->Price = 105;
    order->Qty = 10;
    order->Instrument = "TestInstrument";
    // Submit the order.
    sim.OnNewOrder(order);

    // Run the simulation.
    sim.Run();

    // Verify that the executed order callback was invoked.
    ASSERT_EQ(g_executedOrders.size(), 1u);
    EXPECT_EQ(g_executedOrders[0]->Id, 1);
    EXPECT_EQ(g_executedOrders[0]->State, OrderState::Filled);

    ASSERT_EQ(g_mdTrades.size(), 1u);
    EXPECT_GE(g_mdTrades[0]->LocalTimestamp, 30);

    // Clean up.
    delete order;
}

TEST(SimulationTests, LimitBuyOrderExecutionTest) {
    g_executedOrders.clear();
    g_canceledOrders.clear();
    g_newOrders.clear();
    g_mdTrades.clear();
    g_mdL1Updates.clear();

    CSVMarketDataTradesManager dataCollection({"../../data/simulation_test_trades_3.csv"});
    auto row = dataCollection.GetTrades();
    MDRow mdRow(row);
    MarketDataSimulationManager marketDataManager({MDRow{row}});
    Simulation<10> sim(marketDataManager, 10, 5,
        ExecutedOrderCallback, 
        CanceledOrderCallback,
        ReplacedOrderCallback,
        NewOrderCallback,
        MDTradeCallback,
        MDL1UpdateCallback,
        MDCustomUpdateCallback);

    OrderPtr order = new Order();
    order->Id = 101;
    order->OrderSide = Side::Buy;
    order->Type = OrderType::Limit;
    order->Price = 100;
    order->Qty = 50;
    order->Instrument = "TestInstrument";
    sim.OnNewOrder(order);

    sim.Run();

    ASSERT_EQ(g_executedOrders.size(), 1u);
    EXPECT_EQ(g_executedOrders[0]->Id, 101);
    EXPECT_EQ(g_executedOrders[0]->State, OrderState::Filled);

    delete order;
}

TEST(SimulationTests, LimitSellOrderExecutionTest) {
    g_executedOrders.clear();
    g_canceledOrders.clear();
    g_newOrders.clear();
    g_mdTrades.clear();
    g_mdL1Updates.clear();

    CSVMarketDataTradesManager dataCollection({"../../data/simulation_test_trades_4.csv"});
    auto row = dataCollection.GetTrades();
    MDRow mdRow(row);
    MarketDataSimulationManager marketDataManager({MDRow{row}});
    Simulation<10> sim(marketDataManager, 10, 5,
        ExecutedOrderCallback, 
        CanceledOrderCallback,
        ReplacedOrderCallback,
        NewOrderCallback,
        MDTradeCallback,
        MDL1UpdateCallback,
        MDCustomUpdateCallback);

    OrderPtr order = new Order();
    order->Id = 202;
    order->OrderSide = Side::Sell;
    order->Type = OrderType::Limit;
    order->Price = 100;
    order->Qty = 30;
    order->Instrument = "TestInstrument";

    sim.OnNewOrder(order);
    sim.Run();

    ASSERT_EQ(g_executedOrders.size(), 1u);
    EXPECT_EQ(g_newOrders.size(), 1);
    EXPECT_EQ(g_executedOrders[0]->Price, 100);
    EXPECT_EQ(g_executedOrders[0]->State, OrderState::Filled);

    delete order;
}

TEST(SimulationTests, LimitBuyOrderNotExecutedTest) {
    g_executedOrders.clear();
    g_canceledOrders.clear();
    g_newOrders.clear();
    g_mdTrades.clear();
    g_mdL1Updates.clear();

    CSVMarketDataTradesManager dataCollection({"../../data/simulation_test_trades_4.csv"});
    auto row = dataCollection.GetTrades();
    MDRow mdRow(row);
    MarketDataSimulationManager marketDataManager({MDRow{row}});

    Simulation<10> sim(marketDataManager, 10, 5,
        ExecutedOrderCallback, 
        CanceledOrderCallback,
        ReplacedOrderCallback,
        NewOrderCallback,
        MDTradeCallback,
        MDL1UpdateCallback,
        MDCustomUpdateCallback);

    OrderPtr order = new Order();
    order->Id = 303;
    order->OrderSide = Side::Sell;
    order->Type = OrderType::Limit;
    order->Price = 110;
    order->Qty = 10;
    order->Instrument = "TestInstrument";

    sim.OnNewOrder(order);
    sim.Run();

    EXPECT_TRUE(g_executedOrders.empty());
    EXPECT_EQ(g_newOrders.size(), 1);

    delete order;
}

TEST(SimulationTests, LimitSellOrderNotExecutedTest) {
    g_executedOrders.clear();
    g_canceledOrders.clear();
    g_newOrders.clear();
    g_mdTrades.clear();
    g_mdL1Updates.clear();

    CSVMarketDataTradesManager dataCollection({"../../data/simulation_test_trades_3.csv"});
    auto row = dataCollection.GetTrades();
    MDRow mdRow(row);
    MarketDataSimulationManager marketDataManager({MDRow{row}});

    Simulation<10> sim(marketDataManager, 10, 5,
        ExecutedOrderCallback, 
        CanceledOrderCallback,
        ReplacedOrderCallback,
        NewOrderCallback,
        MDTradeCallback,
        MDL1UpdateCallback,
        MDCustomUpdateCallback);

    OrderPtr order = new Order();
    order->Id = 404;
    order->OrderSide = Side::Buy;
    order->Type = OrderType::Limit;
    order->Price = 85;
    order->Qty = 15;
    order->Instrument = "TestInstrument";

    sim.OnNewOrder(order);
    sim.Run();

    EXPECT_TRUE(g_executedOrders.empty());

    delete order;
}

TEST(SimulationTests, LimitBuyOrderCancelTest) {
    g_executedOrders.clear();
    g_canceledOrders.clear();
    g_newOrders.clear();
    g_mdTrades.clear();
    g_mdL1Updates.clear();

    CSVMarketDataTradesManager dataCollection({"../../data/simulation_test_trades_5.csv"});
    auto row = dataCollection.GetTrades();
    MDRow mdRow(row);
    MarketDataSimulationManager marketDataManager({MDRow{row}});

    Simulation<10> sim(marketDataManager, 10, 5,
        ExecutedOrderCallback, 
        CanceledOrderCallback,
        ReplacedOrderCallback,
        NewOrderCallback,
        MDTradeCallback,
        MDL1UpdateCallback,
        MDCustomUpdateCallback);

    OrderPtr order = new Order();
    order->Id = 101;
    order->OrderSide = Side::Sell;
    order->Type = OrderType::Limit;
    order->Price = 150;
    order->Qty = 50;
    order->Instrument = "TestInstrument";

    sim.OnNewOrder(order);

    sim.Run();

    EXPECT_EQ(g_executedOrders.size(), 0);

    delete order;
}

TEST(SimulationTests, ExecuteOrderMultiinstrumentTest) {
    g_executedOrders.clear();
    g_canceledOrders.clear();
    g_newOrders.clear();
    g_mdTrades.clear();
    g_mdL1Updates.clear();

    CSVMarketDataTradesManager dataCollection_1({"../../data/simulation_test_trades_5.csv"});
    CSVMarketDataTradesManager dataCollection_2({"../../data/simulation_test_trades_6.csv"});
    auto row1 = dataCollection_1.GetTrades();
    auto row2 = dataCollection_2.GetTrades();
    MarketDataSimulationManager marketDataManager({MDRow{row1}, MDRow{row2}});

    Simulation<10> sim(marketDataManager, 0, 0,
        ExecutedOrderCallback, 
        CanceledOrderCallback,
        ReplacedOrderCallback,
        NewOrderCallback,
        MDTradeCallback,
        MDL1UpdateCallback,
        MDCustomUpdateCallback);

    OrderPtr order1 = new Order();
    order1->Id = 1;
    order1->OrderSide = Side::Buy;
    order1->Type = OrderType::Market;
    order1->Price = 101;
    order1->Qty = 10;
    order1->Instrument = "TestInstrument";
    // Submit the order.
    sim.OnNewOrder(order1);

    OrderPtr order2 = new Order();
    order2->Id = 1;
    order2->OrderSide = Side::Buy;
    order2->Type = OrderType::Market;
    order2->Price = 101;
    order2->Qty = 10;
    order2->Instrument = "TestInstrument2";

    sim.OnNewOrder(order2);

    sim.Run();

    ASSERT_EQ(g_executedOrders.size(), 2u);
    EXPECT_EQ(g_executedOrders[0]->Id, 1);
    EXPECT_EQ(g_executedOrders[0]->LastExecPrice, 105);
    EXPECT_EQ(g_executedOrders[0]->LastReportTimestamp, 15);
    EXPECT_EQ(g_executedOrders[0]->State, OrderState::Filled);

    EXPECT_EQ(g_executedOrders[1]->Id, 1);
    EXPECT_EQ(g_executedOrders[1]->LastExecPrice, 105);
    EXPECT_EQ(g_executedOrders[1]->LastReportTimestamp, 16);
    EXPECT_EQ(g_executedOrders[1]->Instrument, "TestInstrument2");
    EXPECT_EQ(g_executedOrders[1]->State, OrderState::Filled);

    ASSERT_EQ(g_mdTrades.size(), 9u);
    EXPECT_GE(g_mdTrades[0]->LocalTimestamp, 15);

    delete order1;
    delete order2;
}

TEST(SimulationTests, MDCustomUpdatesForwardingTest) {
    g_executedOrders.clear();
    g_canceledOrders.clear();
    g_newOrders.clear();
    g_mdTrades.clear();
    g_mdL1Updates.clear();

    std::vector<MDCustomUpdate> updates1(5, MDCustomUpdate());
    std::vector<MDCustomUpdate> updates2(5, MDCustomUpdate());
    for (int i = 0; i < updates1.size(); ++i)
        updates1[i].EventTimestamp = i*3;
    for (int i = 0; i < updates2.size(); ++i)
        updates2[i].EventTimestamp = i * 3 + 1;

    MarketDataSimulationManager marketDataManager({MDRow{updates1}, MDRow{updates2}});

    Simulation<10> sim(marketDataManager, 0, 0,
                       ExecutedOrderCallback,
                       CanceledOrderCallback,
                       ReplacedOrderCallback,
                       NewOrderCallback,
                       MDTradeCallback,
                       MDL1UpdateCallback,
                       MDCustomUpdateCallback);

    sim.Run();

    EXPECT_EQ(g_mdCustomUpdate.size(), 10);
}

TEST(SimulationTests, MDCustomMultipleUpdatesForwardingTest) {
    g_executedOrders.clear();
    g_canceledOrders.clear();
    g_newOrders.clear();
    g_mdTrades.clear();
    g_mdL1Updates.clear();
    g_mdCustomUpdate.clear();
    g_mdCustomMultipleUpdate.clear();

    std::vector<MDCustomMultipleUpdate> updates1(5, MDCustomMultipleUpdate());
    std::vector<MDCustomMultipleUpdate> updates2(5, MDCustomMultipleUpdate());
    for (int i = 0; i < updates1.size(); ++i)
    {
        updates1[i].EventTimestamp = i*3;
        updates1[i].Payload["open"] = i*3;
        updates1[i].Payload["close"] = i*3 + 1;
    }

    for (int i = 0; i < updates2.size(); ++i)
    {
        updates2[i].EventTimestamp = i * 3 + 1;
        updates2[i].Payload["open"] = i*3 + 1;
        updates2[i].Payload["close"] = i*3 + 2;
    }

    MarketDataSimulationManager marketDataManager({MDRow{updates1}, MDRow{updates2}});

    Simulation<10> sim(marketDataManager, 0, 0,
                       ExecutedOrderCallback,
                       CanceledOrderCallback,
                       ReplacedOrderCallback,
                       NewOrderCallback,
                       MDTradeCallback,
                       MDL1UpdateCallback,
                       MDCustomUpdateCallback,
                       MDCustomMultipleUpdateCallback);

    sim.Run();

    EXPECT_EQ(g_mdCustomMultipleUpdate.size(), 10);

    for(auto& update: g_mdCustomMultipleUpdate)
    {
        EXPECT_EQ(update->Payload["open"], update->EventTimestamp);
        EXPECT_EQ(update->Payload["close"], update->EventTimestamp + 1);
    };

}

TEST(SimulationTests, ExecuteOrderAgainstL1Test) {
    g_executedOrders.clear();
    g_canceledOrders.clear();
    g_newOrders.clear();
    g_mdTrades.clear();
    g_mdL1Updates.clear();

    std::vector<MDL1Update> updates;
    for(int i = 0; i < 10; ++i)
    {
        MDL1Update update;
        update.AskPrice = 100 + i;
        update.BidPrice = 90 - i;
        update.AskQty = 1;
        update.BidQty = 1;
        update.Instrument = "TestInstrument";
        update.EventTimestamp = i;
        updates.push_back(update);
    }

    MarketDataSimulationManager marketDataManager({MDRow(updates)});

    Simulation<10> sim(marketDataManager, 0, 0,
        ExecutedOrderCallback, 
        CanceledOrderCallback,
        ReplacedOrderCallback,
        NewOrderCallback,
        MDTradeCallback,
        MDL1UpdateCallback,
        MDCustomUpdateCallback);

    OrderPtr order = new Order();
    order->Id = 1;
    order->OrderSide = Side::Buy;
    order->Type = OrderType::Market;
    order->Price = 105;
    order->Qty = 10;
    order->Instrument = "TestInstrument";
    // Submit the order.
    sim.OnNewOrder(order);

    auto order_ = new Order();
    order_->Id = 2;
    order_->OrderSide = Side::Sell;
    order_->Type = OrderType::Market;
    order_->Price = 0;
    order_->Qty = 10;
    order_->Instrument = "TestInstrument";

    sim.OnNewOrder(order_);

    sim.Run();

    ASSERT_EQ(g_executedOrders.size(), 2u);
    EXPECT_EQ(g_executedOrders[0]->Id, 1);
    EXPECT_EQ(g_executedOrders[0]->State, OrderState::Filled);
    EXPECT_EQ(g_executedOrders[0]->LastExecPrice, 100);

    EXPECT_EQ(g_executedOrders[1]->Id, 2);
    EXPECT_EQ(g_executedOrders[1]->State, OrderState::Filled);
    EXPECT_EQ(g_executedOrders[1]->LastExecPrice, 90);

    ASSERT_EQ(g_mdL1Updates.size(), 10u);
    EXPECT_GE(g_mdL1Updates[0]->LocalTimestamp, 0);

    delete order;
    delete order_;
}

TEST(SimulationTests, LimitOrderExecutedAgainstL1Test) {
    g_executedOrders.clear();
    g_canceledOrders.clear();
    g_newOrders.clear();
    g_mdTrades.clear();
    g_mdL1Updates.clear();

    std::vector<MDL1Update> updates;
    for(int i = 0; i < 10; ++i)
    {
        MDL1Update update;
        update.AskPrice = 100 + i;
        update.BidPrice = 90 - i;
        update.AskQty = 1;
        update.BidQty = 1;
        update.Instrument = "TestInstrument";
        update.EventTimestamp = i;
        updates.push_back(update);
    }

    MarketDataSimulationManager marketDataManager({MDRow(updates)});

    struct Sim
    {
        int count = 0;
        Simulation<10> sim;
        OrderPtr order1, order2;

        Sim(MarketDataSimulationManager &mdManager)
            : sim(mdManager, 0, 0, ExecutedOrderCallback, CanceledOrderCallback, ReplacedOrderCallback, NewOrderCallback, MDTradeCallback,
                  [this](MDL1UpdatePtr update)
                  { this->onL1Update(update); }, MDCustomUpdateCallback)
        {
        }

        void onL1Update(MDL1UpdatePtr update)
        {
            ++count;
            if (count == 3)
            {
                order1 = new Order();
                order1->Id = 1;
                order1->OrderSide = Side::Buy;
                order1->Type = OrderType::Limit;
                order1->Price = 105;
                order1->Qty = 10;
                order1->Instrument = "TestInstrument";

                order2 = new Order();
                order2->Id = 2;
                order2->OrderSide = Side::Sell;
                order2->Type = OrderType::Limit;
                order2->Price = 80;
                order2->Qty = 10;
                order2->Instrument = "TestInstrument";
                sim.OnNewOrder(order1);
                sim.OnNewOrder(order2);
            }
        }

        void Run()
        {
            sim.Run();
        }
    } sim(marketDataManager);
    sim.Run();
    ASSERT_EQ(g_executedOrders.size(), 2u);
    EXPECT_EQ(g_executedOrders[0]->Id, 1);
    EXPECT_EQ(g_executedOrders[0]->State, OrderState::Filled);
    EXPECT_EQ(g_executedOrders[0]->LastExecPrice, 103);

    EXPECT_EQ(g_executedOrders[1]->Id, 2);
    EXPECT_EQ(g_executedOrders[1]->State, OrderState::Filled);
    EXPECT_EQ(g_executedOrders[1]->LastExecPrice, 87);

    delete sim.order1;
    delete sim.order2;
}

TEST(SimulationTests, LimitOrderCanceledL1Test) {
    g_executedOrders.clear();
    g_canceledOrders.clear();
    g_newOrders.clear();
    g_mdTrades.clear();
    g_mdL1Updates.clear();

    std::vector<MDL1Update> updates;
    for(int i = 0; i < 10; ++i)
    {
        MDL1Update update;
        update.AskPrice = 100 - i;
        update.BidPrice = 90 + i;
        update.AskQty = 1;
        update.BidQty = 1;
        update.Instrument = "TestInstrument";
        update.EventTimestamp = i;
        updates.push_back(update);
    }

    MarketDataSimulationManager marketDataManager({MDRow(updates)});

    struct Sim
    {
        int count = 0;
        Simulation<10> sim;
        OrderPtr order1, order2;
        Sim(MarketDataSimulationManager &mdManager)
            : sim(mdManager, 0, 0, ExecutedOrderCallback, CanceledOrderCallback, ReplacedOrderCallback, NewOrderCallback, MDTradeCallback,
                  // Corrected lambda: capture the parameter and call onL1Update.
                  [this](MDL1UpdatePtr update)
                  { this->onL1Update(update); }, MDCustomUpdateCallback)
        {
        }

        void onL1Update(MDL1UpdatePtr update)
        {
            ++count;
            if (count == 1)
            {
                order1 = new Order();
                order1->Id = 1;
                order1->OrderSide = Side::Sell;
                order1->Type = OrderType::Limit;
                order1->Price = 96;
                order1->Qty = 10;
                order1->Instrument = "TestInstrument";

                order2 = new Order();
                order2->Id = 2;
                order2->OrderSide = Side::Buy;
                order2->Type = OrderType::Limit;
                order2->Price = 94;
                order2->Qty = 10;
                order2->Instrument = "TestInstrument";
                sim.OnNewOrder(order1);
                sim.OnNewOrder(order2);
            }

            if (count == 3)
            {
                sim.OnCancelOrder(order1);
                sim.OnCancelOrder(order2);
            }
        }

        void Run()
        {
            sim.Run();
        }
    } sim(marketDataManager);
    sim.Run();
    ASSERT_EQ(g_executedOrders.size(), 0u);
}