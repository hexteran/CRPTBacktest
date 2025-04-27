#include "market_data_simulation_manager.hpp"
#include "order_execution_manager.hpp"
#include "../utils/circular_buffer.hpp"
template <int QueueSize>
class Simulation
{
public:
    Simulation(MarketDataSimulationManager &marketDataManager,
               Timestamp executionLatency,
               Timestamp marketDataLatency,
               std::function<void(OrderPtr)> executed_order_callback,
               std::function<void(OrderPtr)> canceled_order_callback,
               std::function<void(OrderPtr)> replaced_order_callback,
               std::function<void(OrderPtr)> new_order_callback,
               std::function<void(MDTradePtr)> md_trade_callback,
               std::function<void(MDL1UpdatePtr)> md_l1_callback,
               std::function<void(MDCustomUpdatePtr)> md_custom_update_callback = std::function<void(MDCustomUpdatePtr)>(),
               std::function<void(MDCustomMultipleUpdatePtr)> md_custom_multiple_update_callback = std::function<void(MDCustomMultipleUpdatePtr)>()) : m_marketDataManager(marketDataManager),
                                                                                                                                               m_executionLatency(executionLatency),
                                                                                                                                               m_marketDataLatency(marketDataLatency),
                                                                                                                                               m_canceled_order_callback(canceled_order_callback),
                                                                                                                                               m_replaced_order_callback(replaced_order_callback),
                                                                                                                                               m_executed_order_callback(executed_order_callback),
                                                                                                                                               m_new_order_callback(new_order_callback),
                                                                                                                                               m_md_trade_callback(md_trade_callback),
                                                                                                                                               m_md_l1_callback(md_l1_callback),
                                                                                                                                               m_md_custom_update_callback(md_custom_update_callback),
                                                                                                                                               m_md_custom_multiple_update_callback(md_custom_multiple_update_callback)
    {
    }

    void OnNewOrder(OrderPtr order)
    {
        order->State = OrderState::PendingNew;
        order->CreateTimestamp = m_currentTimestamp;
        m_input_order_queue.PushBack(order);
    }

    void OnCancelOrder(OrderPtr order)
    {
        if(order->State != OrderState::Filled && order->State != OrderState::Canceled)
        {
            order->State = OrderState::PendingCancel;
            m_input_order_cancel_queue.PushBack(order);
        }
    }

    void OnOrderReplace(OrderPtr order, double price, double qty)
    {
        m_input_replaced_orders_queue.PushBack(std::make_tuple(order, price, qty, m_currentTimestamp));
    }

    Timestamp GetCurrentTimestamp()
    {
        return m_currentTimestamp;
    }

    Timestamp GetNextTimestamp()
    {
        return m_nextTimestamp;
    }

    void Run()
    {
        for (auto iter = m_marketDataManager.begin(); iter != m_marketDataManager.end(); ++iter)
        {
            m_currentTimestamp = iter->EventTimestamp;
            if (iter + 1 != m_marketDataManager.end())
                m_nextTimestamp = (iter + 1)->EventTimestamp;
            processInputMessages(*iter);
            processMDTypeSpecificInfo(*iter);
            processOutputQueues(*iter);
        }
    }

private:
    void processMDUpdate(MDTradePtr trade)
    {
        m_output_md_trades_queue.PushBack(trade);
        trade->LocalTimestamp = trade->EventTimestamp + m_marketDataLatency;
        auto result = m_order_exection_manager[trade->Instrument].MatchWithPrice(trade->Price, trade->AggressorSide);

        for (auto &order : result)
            if (order->State != OrderState::PendingCancel && order->State != OrderState::Canceled)
                m_output_executed_orders_queue.PushBack(order);
    }

    void processMDUpdate(MDL1UpdatePtr update)
    {
        m_output_md_l1_updates_queue.PushBack(update);
        update->LocalTimestamp = update->EventTimestamp + m_marketDataLatency;
        auto resultBuy = m_order_exection_manager[update->Instrument].MatchWithPrice(update->AskPrice, Side::Sell);

        for (auto &order : resultBuy)
            if (order->State != OrderState::PendingCancel && order->State != OrderState::Canceled)
                m_output_executed_orders_queue.PushBack(order);

        auto resultSell = m_order_exection_manager[update->Instrument].MatchWithPrice(update->BidPrice, Side::Buy);
        for (auto &order : resultSell)
            if (order->State != OrderState::PendingCancel && order->State != OrderState::Canceled)
                m_output_executed_orders_queue.PushBack(order);
    }

    void processMDUpdate(MDCustomUpdatePtr update)
    {
        m_output_md_custom_updates_queue.PushBack(update);
    }

    void processMDUpdate(MDCustomMultipleUpdatePtr update)
    {
        m_output_md_custom_multiple_updates_queue.PushBack(update);
    }

    void processMDTypeSpecificInfo(MarketDataUpdatePtr update)
    {
        switch (update->Type)
        {
            case MarketDataType::Trade:
            {
                processMDUpdate(MDTradePtr(update));
                return;
            }
            case MarketDataType::L1Update:
            {
                processMDUpdate(MDL1UpdatePtr(update));
                return;
            }
            case MarketDataType::Custom:
            {
                processMDUpdate(MDCustomUpdatePtr(update));
                return;
            }
            case MarketDataType::CustomMultiple:
            {
                processMDUpdate(MDCustomMultipleUpdatePtr(update));
                return;
            }
            default:
            {
                throw std::runtime_error("Data type is not supported");
            }
        }
    }

    void processInputMessages(MarketDataUpdatePtr update)
    {
        while (!m_input_order_queue.Empty() &&
               update->EventTimestamp >= m_input_order_queue.Front()->CreateTimestamp + m_executionLatency)
        {
            auto &order = m_input_order_queue.Front();
            m_order_exection_manager[order->Instrument].AddNewOrder(order);
            m_output_new_orders_queue.PushBack(order);
            m_input_order_queue.PopFront();
        }

        while (!m_input_order_cancel_queue.Empty() &&
               update->EventTimestamp >= m_input_order_cancel_queue.Front()->CreateTimestamp + m_executionLatency)
        {
            auto &order = m_input_order_cancel_queue.Front();
            if(order->State != OrderState::Filled)
            {
                m_order_exection_manager[order->Instrument].CancelOrder(order);
            }
            m_output_canceled_orders_queue.PushBack(order);
            m_input_order_cancel_queue.PopFront();
        }

        while (!m_input_replaced_orders_queue.Empty() &&
               update->EventTimestamp >= std::get<3>(m_input_replaced_orders_queue.Front()) + m_executionLatency)
        {
            auto &record = m_input_replaced_orders_queue.Front();
            if (std::get<0>(record)->State != OrderState::Active)
            {
                m_input_replaced_orders_queue.PopFront();
                continue;
            }
            m_order_exection_manager[std::get<0>(record)->Instrument].ReplaceOrder(std::get<0>(record), std::get<1>(record), std::get<2>(record));
            m_output_replaced_orders_queue.PushBack(std::make_tuple(std::get<0>(record), std::get<3>(record)));
            m_input_replaced_orders_queue.PopFront();
        }
    }

    void processOutputQueues(MarketDataUpdatePtr update)
    {
        while (!m_output_new_orders_queue.Empty() &&
               update->EventTimestamp >= m_output_new_orders_queue.Front()->CreateTimestamp + m_executionLatency)
        {
            auto &order = m_output_new_orders_queue.Front();
            order->LastReportTimestamp = m_currentTimestamp;
            m_output_new_orders_queue.Front()->State = OrderState::Active;
            m_new_order_callback(m_output_new_orders_queue.Front());
            m_output_new_orders_queue.PopFront();
        }

        while (!m_output_canceled_orders_queue.Empty() &&
               update->EventTimestamp >= m_output_canceled_orders_queue.Front()->CreateTimestamp + 2 * m_executionLatency)
        {
            auto &order = m_output_canceled_orders_queue.Front();
            if(order->State != OrderState::Filled)
            {
                order->LastReportTimestamp = m_currentTimestamp;
                order->State = OrderState::Canceled;
                m_canceled_order_callback(order);
            }
            m_output_canceled_orders_queue.PopFront();
        }

        while (!m_output_replaced_orders_queue.Empty() &&
               update->EventTimestamp >= std::get<1>(m_output_replaced_orders_queue.Front()) + 2 * m_executionLatency)
        {
            auto &order = std::get<0>(m_output_replaced_orders_queue.Front());
            if (order->State != OrderState::Active)
            {
                continue;
                m_output_replaced_orders_queue.PopFront();
            }
            order->LastReportTimestamp = m_currentTimestamp;
            order->State = OrderState::Active;
            m_replaced_order_callback(order);
            m_output_replaced_orders_queue.PopFront();
        }

        while (!m_output_executed_orders_queue.Empty() &&
               update->EventTimestamp >= m_output_executed_orders_queue.Front()->CreateTimestamp + 2 * m_executionLatency)
        {
            auto &order = m_output_executed_orders_queue.Front();
            order->LastReportTimestamp = m_currentTimestamp;
            order->State = OrderState::Filled;
            m_executed_order_callback(order);
            m_output_executed_orders_queue.PopFront();
        }

        while (!m_output_md_trades_queue.Empty() &&
               update->EventTimestamp >= m_output_md_trades_queue.Front()->LocalTimestamp)
        {
            m_md_trade_callback(m_output_md_trades_queue.Front());
            m_output_md_trades_queue.PopFront();
        }

        while (!m_output_md_l1_updates_queue.Empty() &&
               update->EventTimestamp >= m_output_md_l1_updates_queue.Front()->LocalTimestamp)
        {
            m_md_l1_callback(m_output_md_l1_updates_queue.Front());
            m_output_md_l1_updates_queue.PopFront();
        }

        while (!m_output_md_custom_updates_queue.Empty() &&
               update->EventTimestamp >= m_output_md_custom_updates_queue.Front()->EventTimestamp + m_marketDataLatency)
        {
            m_md_custom_update_callback(m_output_md_custom_updates_queue.Front());
            m_output_md_custom_updates_queue.PopFront();
        }

        while (!m_output_md_custom_multiple_updates_queue.Empty() &&
               update->EventTimestamp >= m_output_md_custom_multiple_updates_queue.Front()->EventTimestamp + m_marketDataLatency)
        {
            m_md_custom_multiple_update_callback(m_output_md_custom_multiple_updates_queue.Front());
            m_output_md_custom_multiple_updates_queue.PopFront();
        }
    }

private:
    MarketDataSimulationManager &m_marketDataManager;
    CircularBuffer<OrderPtr, QueueSize> m_input_order_queue;
    CircularBuffer<OrderPtr, QueueSize> m_input_order_cancel_queue;
    CircularBuffer<std::tuple<OrderPtr, double, double, Timestamp>, QueueSize> m_input_replaced_orders_queue;
    CircularBuffer<OrderPtr, QueueSize> m_output_new_orders_queue;
    CircularBuffer<OrderPtr, QueueSize> m_output_executed_orders_queue;
    CircularBuffer<OrderPtr, QueueSize> m_output_canceled_orders_queue;
    CircularBuffer<std::tuple<OrderPtr, Timestamp>, QueueSize> m_output_replaced_orders_queue;
    CircularBuffer<MDTradePtr, QueueSize> m_output_md_trades_queue;
    CircularBuffer<MDCustomUpdatePtr, QueueSize> m_output_md_custom_updates_queue;
    CircularBuffer<MDCustomMultipleUpdatePtr, QueueSize> m_output_md_custom_multiple_updates_queue;
    CircularBuffer<MDL1UpdatePtr, QueueSize> m_output_md_l1_updates_queue;

    std::unordered_map<std::string, OrderExecutionManager> m_order_exection_manager;

    std::function<void(OrderPtr)> m_executed_order_callback;
    std::function<void(OrderPtr)> m_canceled_order_callback;
    std::function<void(OrderPtr)> m_replaced_order_callback;
    std::function<void(OrderPtr)> m_new_order_callback;
    std::function<void(MDTradePtr)> m_md_trade_callback;
    std::function<void(MDL1UpdatePtr)> m_md_l1_callback;
    std::function<void(MDCustomUpdatePtr)> m_md_custom_update_callback;
    std::function<void(MDCustomMultipleUpdatePtr)> m_md_custom_multiple_update_callback;

    Timedelta m_executionLatency{0}, m_marketDataLatency{0};
    Timestamp m_currentTimestamp{0}, m_nextTimestamp{0};
};