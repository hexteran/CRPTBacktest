#include "../core/simulation.hpp"
#include <pybind11/stl.h>
#include <pybind11/complex.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;

class PyDataStorage
{
public:
    void AddVMDTrades(
                    const std::string& rowName,
                    const std::vector<Timestamp>& timestamps,
                    const std::vector<double>& prices,
                    const std::vector<double>& qtys,
                    const std::vector<Side>& sides,
                    const std::vector<std::string>& instruments)
    {
        auto& row = m_trades[rowName] = std::vector<MDTrade>(timestamps.size(), MDTrade());
        for(int i = 0; i < timestamps.size(); ++i)
        {
            row[i].EventTimestamp = timestamps[i];
            row[i].Instrument = instruments[i];
            row[i].Price = prices[i];
            row[i].Qty = qtys[i];
            row[i].AggressorSide = sides[i];
        }
    }

    void AddVMDL1Updtes(
                    const std::string& rowName,
                    const std::vector<Timestamp>& timestamps,
                    const std::vector<double>& askPrices,
                    const std::vector<double>& askQtys,
                    const std::vector<double>& bidPrices,
                    const std::vector<double>& bidQtys,
                    const std::vector<std::string>& instruments)
    {
        auto& row = m_l1_updates[rowName] = std::vector<MDL1Update>(timestamps.size(), MDL1Update());
        for(int i = 0; i < timestamps.size(); ++i)
        {
            row[i].EventTimestamp = timestamps[i];
            row[i].Instrument = instruments[i];
            row[i].AskPrice = askPrices[i];
            row[i].AskQty = askQtys[i];
            row[i].BidPrice = bidPrices[i];
            row[i].BidQty = bidQtys[i];
        }
    }

    void AddVMDCustomUpdates(
        const std::string &rowName,
        const std::vector<Timestamp> &timestamps,
        const std::vector<std::string> &texts,
        const std::vector<double>& payloads)
    {
        auto &row = m_customUpdates[rowName] = std::vector<MDCustomUpdate>(timestamps.size(), MDCustomUpdate());
        for (int i = 0; i < timestamps.size(); ++i)
        {
            row[i].EventTimestamp = timestamps[i];
            row[i].Text = texts[i];
            row[i].Payload = payloads[i];
        }
    }

    void AddVMDCustomMultipleUpdates(
        const std::string &rowName,
        const std::vector<Timestamp> &timestamps,
        const std::vector<std::string> &texts,
        const std::vector<std::unordered_map<std::string, double>>& payloads)
    {
        auto &row = m_customMultipleUpdates[rowName] = std::vector<MDCustomMultipleUpdate>(timestamps.size(), MDCustomMultipleUpdate());
        for (int i = 0; i < timestamps.size(); ++i)
        {
            row[i].EventTimestamp = timestamps[i];
            row[i].Text = texts[i];
            row[i].Payload = payloads[i];
        }
    }

    void AddMDTrades(const std::unordered_map<std::string, std::vector<MDTrade>>& trades)
    {
        for(auto& [id, data]: trades)
            m_trades[id] = data;
    }

    void AddMDL1Updates(const std::unordered_map<std::string, std::vector<MDL1Update>>& l1Updates)
    {
        for(auto& [id, data]: l1Updates)
            m_l1_updates[id] = data;
    }

    void AddMDCustomUpdates(const std::unordered_map<std::string, std::vector<MDCustomUpdate>>& updates)
    {
        for(auto& [id, data]: updates)
            m_customUpdates[id] = data;
    }

    void AddMDCustomMultipleUpdates(const std::unordered_map<std::string, std::vector<MDCustomMultipleUpdate>>& updates)
    {
        for(auto& [id, data]: updates)
        {
            m_customMultipleUpdates[id] = data;
        }
    }

    std::unordered_map<std::string, std::vector<MDTrade>>& GetMDTrades()
    {
        return m_trades;
    }

    std::unordered_map<std::string, std::vector<MDL1Update>>& GetMDL1Updates()
    {
        return m_l1_updates;
    }


    std::unordered_map<std::string, std::vector<MDCustomUpdate>>& GetMDCustomUpdates()
    {
        return m_customUpdates;
    }

    std::unordered_map<std::string, std::vector<MDCustomMultipleUpdate>>& GetMDCustomMultipleUpdates()
    {
        return m_customMultipleUpdates;
    }

private:
    std::unordered_map<std::string, std::vector<MDTrade>> m_trades;
    std::unordered_map<std::string, std::vector<MDL1Update>> m_l1_updates;
    std::unordered_map<std::string, std::vector<MDCustomUpdate>> m_customUpdates;
    std::unordered_map<std::string, std::vector<MDCustomMultipleUpdate>> m_customMultipleUpdates;
};

class PyStrategy
{
public:
    PyStrategy(
        PyDataStorage& storage,
        Timestamp executionLatency, 
        Timestamp marketDataLatency,
        std::function<void(OrderPtr)> executed_order_callback,
        std::function<void(OrderPtr)> canceled_order_callback,
        std::function<void(OrderPtr)> replaced_order_callback,
        std::function<void(OrderPtr)> new_order_callback,
        std::function<void(MDTradePtr)> md_trade_callback,
        std::function<void(MDL1UpdatePtr)> md_l1_update_callback,
        std::function<void(MDCustomUpdatePtr)> md_custom_update_callback,
        std::function<void(MDCustomMultipleUpdatePtr)> md_custom_multiple_update_callback):
        m_simulation(m_marketDataManager,
            executionLatency, 
            marketDataLatency,
            executed_order_callback,
            canceled_order_callback,
            replaced_order_callback,
            new_order_callback,
            md_trade_callback,
            md_l1_update_callback,
            md_custom_update_callback,
            md_custom_multiple_update_callback),
        m_storage(storage),
        m_destroyStorage(false)
    {}

    PyStrategy(
        Timestamp executionLatency, 
        Timestamp marketDataLatency,
        std::function<void(OrderPtr)> executed_order_callback,
        std::function<void(OrderPtr)> canceled_order_callback,
        std::function<void(OrderPtr)> replaced_order_callback,
        std::function<void(OrderPtr)> new_order_callback,
        std::function<void(MDTradePtr)> md_trade_callback,
        std::function<void(MDL1UpdatePtr)> md_l1_update_callback,
        std::function<void(MDCustomUpdatePtr)> md_custom_update_callback,
        std::function<void(MDCustomMultipleUpdatePtr)> md_custom_multiple_update_callback):
        m_simulation(m_marketDataManager,
            executionLatency, 
            marketDataLatency,
            executed_order_callback,
            canceled_order_callback,
            replaced_order_callback,
            new_order_callback,
            md_trade_callback,
            md_l1_update_callback,
            md_custom_update_callback,
            md_custom_multiple_update_callback),
        m_storage(*(new PyDataStorage())),
        m_destroyStorage(true)
    {}
    
    //void AddMDTrades(const std::unordered_map<std::string, std::vector<>>& trades)
    void CommitData()
    {
        ClearDataManager();
        for(auto& [id, data]: m_storage.GetMDTrades())
            m_marketDataManager.AddRow(MDRow{data, id});
        for(auto& [id, data]: m_storage.GetMDL1Updates())
            m_marketDataManager.AddRow(MDRow{data, id});
        for(auto& [id, data]: m_storage.GetMDCustomUpdates())
            m_marketDataManager.AddRow(MDRow{data, id});
        for(auto& [id, data]: m_storage.GetMDCustomMultipleUpdates())
            m_marketDataManager.AddRow(MDRow{data, id});
    }

    void Run()
    {
        m_simulation.Run();
    }

    void ClearDataManager()
    {
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

    void AddMDTrades(const std::unordered_map<std::string, std::vector<MDTrade>>& trades)
    {
        m_storage.AddMDTrades(trades);
    }

    void AddMDL1Updates(const std::unordered_map<std::string, std::vector<MDL1Update>>& l1Updates)
    {
        m_storage.AddMDL1Updates(l1Updates);
    }

    void AddMDCustomUpdates(const std::unordered_map<std::string, std::vector<MDCustomUpdate>>& updates)
    {
        m_storage.AddMDCustomUpdates(updates);
    }

    void AddMDCustomMultipleUpdates(const std::unordered_map<std::string, std::vector<MDCustomMultipleUpdate>>& updates)
    {
        m_storage.AddMDCustomMultipleUpdates(updates);
    }

    ~PyStrategy()
    {
        if (m_destroyStorage)
            delete &m_storage;
    }

private:
    PyDataStorage& m_storage;
    bool m_destroyStorage{false};
    MarketDataSimulationManager m_marketDataManager;
    Simulation<1000000> m_simulation;
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

    py::class_<MDTrade>(m, "MDTrade")
        .def(py::init(), "Constructor for MDTrade")
        .def(py::init<const MDTrade &>())
        .def_readwrite("AggressorSide", &MDTrade::AggressorSide)
        .def_readwrite("EventTimestamp", &MDTrade::EventTimestamp)
        .def_readwrite("LocalTimestamp", &MDTrade::LocalTimestamp)
        .def_readwrite("Instrument", &MDTrade::Instrument)
        .def_readwrite("Price", &MDTrade::Price)
        .def_readwrite("Qty", &MDTrade::Qty);

    py::class_<MDL1Update>(m, "MDL1Update")
        .def(py::init(), "Constructor for MDL1Update")
        .def(py::init<const MDL1Update &>())
        .def_readwrite("AskPrice", &MDL1Update::AskPrice)
        .def_readwrite("AskQty", &MDL1Update::AskQty)
        .def_readwrite("BidPrice", &MDL1Update::BidPrice)
        .def_readwrite("BidQty", &MDL1Update::BidQty)
        .def_readwrite("EventTimestamp", &MDL1Update::EventTimestamp)
        .def_readwrite("LocalTimestamp", &MDL1Update::LocalTimestamp)
        .def_readwrite("Instrument", &MDL1Update::Instrument);

    py::class_<MDCustomUpdate>(m, "MDCustomUpdate")
        .def(py::init(), "Constructor for MDCustomUpdate")
        .def(py::init<const MDCustomUpdate &>())
        .def_readwrite("Text", &MDCustomUpdate::Text)
        .def_readwrite("Payload", &MDCustomUpdate::Payload)
        .def_readwrite("EventTimestamp", &MDCustomUpdate::EventTimestamp);

    py::class_<MDCustomMultipleUpdate>(m, "MDCustomMultipleUpdate")
        .def(py::init(), "Constructor for MDCustomMultipleUpdate")
        .def(py::init<const MDCustomMultipleUpdate &>())
        .def_readwrite("Text", &MDCustomMultipleUpdate::Text)
        .def_readwrite("Payload", &MDCustomMultipleUpdate::Payload)
        .def_readwrite("EventTimestamp", &MDCustomMultipleUpdate::EventTimestamp);

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

    py::class_<PyDataStorage>(m, "PyDataStorage")
        .def(py::init<>())
        .def("add_v_md_trades", &PyDataStorage::AddVMDTrades, "Add dict of md trades")
        .def("add_v_md_l1_updates", &PyDataStorage::AddVMDL1Updtes, "Add dict of md l1 updates")
        .def("add_v_md_custom_updates", &PyDataStorage::AddVMDCustomUpdates, "Add dict of md custom updates")
        .def("add_v_md_custom_multiple_updates", &PyDataStorage::AddVMDCustomMultipleUpdates, "Add dict of md custom multiple updates")
        .def("add_md_trades", &PyDataStorage::AddMDTrades, "Add dict of md trades")
        .def("add_md_l1_updates", &PyDataStorage::AddMDL1Updates, "Add dict of md l1 updates")
        .def("add_md_custom_updates", &PyDataStorage::AddMDCustomUpdates, "Add dict of md custom updates")
        .def("add_md_custom_multiple_updates", &PyDataStorage::AddMDCustomMultipleUpdates, "Add dict of md custom multiple updates");
        
    py::class_<PyStrategy>(m, "PyStrategy")
        .def(py::init<
                PyDataStorage&,
                Timestamp,                     // executionLatency
                Timestamp,                     // marketDataLatency
                std::function<void(OrderPtr)>, // executed_order_callback
                std::function<void(OrderPtr)>, // canceled_order_callback
                std::function<void(OrderPtr)>, // replaced_order_callback
                std::function<void(OrderPtr)>, // new_order_callback
                std::function<void(MDTradePtr)>,// md_trade_callback
                std::function<void(MDL1UpdatePtr)>,
                std::function<void(MDCustomUpdatePtr)>,
                std::function<void(MDCustomMultipleUpdatePtr)>
            >(),
            py::arg("dataStorage"),
            py::arg("executionLatency"),
            py::arg("marketDataLatency"),
            py::arg("executed_order_callback"),
            py::arg("canceled_order_callback"),
            py::arg("replaced_order_callback"),
            py::arg("new_order_callback"),
            py::arg("md_trade_callback"),
            py::arg("md_l1_update_callback"),
            py::arg("md_custom_update_callback"),
            py::arg("md_custom_multiple_update_callback"))

        .def(py::init<
                Timestamp,                     // executionLatency
                Timestamp,                     // marketDataLatency
                std::function<void(OrderPtr)>, // executed_order_callback
                std::function<void(OrderPtr)>, // canceled_order_callback
                std::function<void(OrderPtr)>, // replaced_order_callback
                std::function<void(OrderPtr)>, // new_order_callback
                std::function<void(MDTradePtr)>,// md_trade_callback
                std::function<void(MDL1UpdatePtr)>,
                std::function<void(MDCustomUpdatePtr)>,
                std::function<void(MDCustomMultipleUpdatePtr)>
            >(),
            py::arg("executionLatency"),
            py::arg("marketDataLatency"),
            py::arg("executed_order_callback"),
            py::arg("canceled_order_callback"),
            py::arg("replaced_order_callback"),
            py::arg("new_order_callback"),
            py::arg("md_trade_callback"),
            py::arg("md_l1_update_callback"),
            py::arg("md_custom_update_callback"),
            py::arg("md_custom_multiple_update_callback")
        )
        .def("run", &PyStrategy::Run, "Run simulation")
        .def("send_order", &PyStrategy::SendOrder, "Send an order to the simulation")
        .def("cancel_order", &PyStrategy::CancelOrder, "Cancel an order in the simulation")
        .def("add_md_trades", &PyStrategy::AddMDTrades, "Add dict of md trades")
        .def("add_md_l1_updates", &PyStrategy::AddMDL1Updates, "Add dict of md l1 updates")
        .def("add_md_custom_updates", &PyStrategy::AddMDCustomUpdates, "Add dict of md custom updates")
        .def("add_md_custom_multiple_updates", &PyStrategy::AddMDCustomMultipleUpdates, "Add dict of md custom multiple updates")
        .def("commit_data", &PyStrategy::CommitData, "Commits data to a market data manager")
        .def("clear_data_manager", &PyStrategy::ClearDataManager, "Clears market data from manager");
}