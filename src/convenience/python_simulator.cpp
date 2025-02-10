#include "../core/simulation.hpp"
#include <pybind11/stl.h>
#include <pybind11/complex.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;

class PyMarketDataTradesManager: public IMarketDataTradesManager
{
public:
    PyMarketDataTradesManager(const std::vector<MDTrade>& data): 
        m_data(data)
    {};
    std::vector<MDTrade>::iterator begin() final
    {
        return m_data.begin();
    }

    std::vector<MDTrade>::iterator end() final
    {
        return m_data.end();
    }
private:
    std::vector<MDTrade> m_data;
};

class PyStrategy
{
public:
    PyStrategy(
        Timestamp executionLatency, 
        Timestamp marketDataLatency,
        std::function<void(OrderPtr)> executed_order_callback,
        std::function<void(OrderPtr)> canceled_order_callback,
        std::function<void(OrderPtr)> replaced_order_callback,
        std::function<void(OrderPtr)> new_order_callback,
        std::function<void(MDTradePtr)> md_trade_callback):
        m_simulation(m_marketDataManager, 
            executionLatency, 
            marketDataLatency,
            executed_order_callback,
            canceled_order_callback,
            replaced_order_callback,
            new_order_callback,
            md_trade_callback)
    {}

    void AddMDTrades(const std::unordered_map<std::string, std::vector<MDTrade>>& trades)
    {
        for(auto& [id, data]: trades)
            m_trades[id] = data;
    }

    void AddMDCustomUpdates(const std::unordered_map<std::string, std::vector<MDCustomUpdate>>& updates)
    {
        for(auto& [id, data]: updates)
            m_customUpdates[id] = data;
    }

    void Run()
    {
        for(auto& [id, data]: m_trades)
            m_marketDataManager.AddRow(MDRow{data, id});
        for(auto& [id, data]: m_customUpdates)
            m_marketDataManager.AddRow(MDRow{data, id});

        m_simulation.Run();

        m_marketDataManager.Clear();
    }

    void SendOrder(OrderPtr order)
    {
        m_simulation.OnNewOrder(order);
    }

    void CancelOrder(OrderPtr order)
    {
        m_simulation.OnCancelOrder(order);
    }

private:
    std::unordered_map<std::string, std::vector<MDTrade>> m_trades;
    std::unordered_map<std::string, std::vector<MDCustomUpdate>> m_customUpdates;
    MarketDataSimulationManager m_marketDataManager;
    Simulation<10000> m_simulation;
};

// Note the module name is "my_module"
PYBIND11_MODULE(python_simulator, m) {
    m.doc() = "A simple example module";

    py::enum_<Side>(m, "Side")
        .value("Buy", Side::Buy)
        .value("Sell", Side::Sell);

    py::enum_<MarketDataType>(m, "MarketDataType")
        .value("Trade", MarketDataType::Trade)
        .value("L1Update", MarketDataType::L1Update)
        .value("L2Update", MarketDataType::L2Update);

    py::enum_<OrderType>(m, "OrderType")
        .value("Limit", OrderType::Limit)
        .value("Market", OrderType::Market);

    py::enum_<OrderState>(m, "OrderState")
        .value("Active", OrderState::Active)
        .value("Rejected", OrderState::Rejected)
        .value("Canceled", OrderState::Canceled)
        .value("PendingNew", OrderState::PendingNew)
        .value("PendingCancel", OrderState::PendingCancel)
        .value("Filled", OrderState::Filled)
        .value("PartiallyFilled", OrderState::PartiallyFilled);

    py::class_<PyMarketDataTradesManager>(m, "PyMarketDataTradesManager")
        .def(py::init<const std::vector<MDTrade>&>(), 
             "Create a new PyMarketDataTradesManager from a list of MDTrade objects",
             py::arg("data"));

    py::class_<MDTrade>(m, "MDTrade")
        .def(py::init(), "Constructor for MDTrade")
        .def(py::init<const MDTrade &>())
        .def_readwrite("AggressorSide", &MDTrade::AggressorSide)
        .def_readwrite("EventTimestamp", &MDTrade::EventTimestamp)
        .def_readwrite("Instrument", &MDTrade::Instrument)
        .def_readwrite("Price", &MDTrade::Price)
        .def_readwrite("Qty", &MDTrade::Qty);

    py::class_<MDCustomUpdate>(m, "MDCustomUpdate")
        .def(py::init(), "Constructor for MDCustomUpdate")
        .def(py::init<const MDCustomUpdate &>())
        .def_readwrite("EventTimestamp", &MDTrade::EventTimestamp);

    py::class_<Order>(m, "Order")
        .def(py::init(), "Constructor for MDTrade")
        .def_readwrite("AggressorSide", &Order::Id)
        .def_readwrite("EventTimestamp", &Order::ClOrdId)
        .def_readwrite("Instrument", &Order::Instrument)
        .def_readwrite("State", &Order::State)
        .def_readwrite("Price", &Order::Type)
        .def_readwrite("Qty", &Order::OrderSide)
        .def_readwrite("Instrument", &Order::Instrument)
        .def_readwrite("OrderSide", &Order::OrderSide)
        .def_readwrite("Type", &Order::Type)
        .def_readwrite("Text", &Order::Text)
        .def_readwrite("Price", &Order::Price)
        .def_readwrite("Qty", &Order::Qty)
        .def_readwrite("FilledQty", &Order::FilledQty)
        .def_readwrite("LastExecPrice", &Order::LastExecPrice)
        .def_readwrite("CreateTimestamp", &Order::CreateTimestamp)
        .def_readwrite("LastReportTimestamp", &Order::LastReportTimestamp)
        .def("to_string", &Order::ToString, "Return the order details as a string");

    py::class_<PyStrategy>(m, "PyStrategy")
        .def(py::init<
                Timestamp,                     // executionLatency
                Timestamp,                     // marketDataLatency
                std::function<void(OrderPtr)>, // executed_order_callback
                std::function<void(OrderPtr)>, // canceled_order_callback
                std::function<void(OrderPtr)>, // replaced_order_callback
                std::function<void(OrderPtr)>, // new_order_callback
                std::function<void(MDTradePtr)>// md_trade_callback
            >(),
            py::arg("data"),
            py::arg("executionLatency"),
            py::arg("marketDataLatency"),
            py::arg("executed_order_callback"),
            py::arg("canceled_order_callback"),
            py::arg("replaced_order_callback"),
            py::arg("new_order_callback"),
            py::arg("md_trade_callback")
        )
        .def("run", &PyStrategy::Run, "Run simulation")
        .def("send_order", &PyStrategy::SendOrder, "Send an order to the simulation")
        .def("cancel_order", &PyStrategy::CancelOrder, "Cancel an order in the simulation")
        .def("add_md_trades", &PyStrategy::AddMDTrades, "Add dict of md trades")
        .def("add_md_custom_updates", &PyStrategy::AddMDCustomUpdates, "Add dict of md custom updates");
}