#include "market_data_simulation_manager.hpp"
#include "order_execution_manager.hpp"
#include "../utils/circular_buffer.hpp"

template <int QueueSize>
class Simulation
{
public:
    Simulation(MarketDataSimulationManager& marketDataManager,
               Timestamp executionLatency,
               Timestamp marketDataLatency,
               std::function<void(OrderPtr)> executed_order_callback,
               std::function<void(OrderPtr)> canceled_order_callback,
               std::function<void(OrderPtr)> replaced_order_callback,
               std::function<void(OrderPtr)> new_order_callback,
               std::function<void(MDTradePtr)> md_trade_callback) : m_marketDataManager(marketDataManager),
                                                                    m_executionLatency(executionLatency),
                                                                    m_marketDataLatency(marketDataLatency),
                                                                    m_canceled_order_callback(canceled_order_callback),
                                                                    m_replaced_order_callback(replaced_order_callback),
                                                                    m_executed_order_callback(executed_order_callback),
                                                                    m_new_order_callback(new_order_callback),
                                                                    m_md_trade_callback(md_trade_callback)
    {
    }

    void OnNewOrder(OrderPtr order)
    {
        order->CreateTimestamp = m_currentTimestamp;
        m_input_order_queue.PushBack(order);
    }

    void OnCancelOrder(OrderPtr order)
    {
        m_output_canceled_orders_queue.PushBack(order);
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
        auto result = m_order_exection_manager.MatchWithPrice(trade->Price, trade->AggressorSide);

        for (auto &order : result)
            if (order->State != OrderState::PendingCancel && order->State != OrderState::Canceled)
                m_output_executed_orders_queue.PushBack(order);
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
            case MarketDataType::Custom:
            {
                break;
            }
        }
    }

    void processInputMessages(MarketDataUpdatePtr update)
    {
        while (!m_input_order_queue.Empty() &&
               update->EventTimestamp >= m_input_order_queue.Front()->CreateTimestamp + m_executionLatency)
        {
            auto &order = m_input_order_queue.Front();
            m_order_exection_manager.AddNewOrder(order);
            m_output_new_orders_queue.PushBack(order);
            m_input_order_queue.PopFront();
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
            m_order_exection_manager.ReplaceOrder(std::get<0>(record), std::get<1>(record), std::get<2>(record));
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
            order->LastReportTimestamp = m_currentTimestamp;
            order->State = OrderState::Canceled;
            m_canceled_order_callback(order);
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
    }

private:
    MarketDataSimulationManager &m_marketDataManager;
    CircularBuffer<OrderPtr, QueueSize> m_input_order_queue;
    CircularBuffer<std::tuple<OrderPtr, double, double, Timestamp>, QueueSize> m_input_replaced_orders_queue;
    CircularBuffer<OrderPtr, QueueSize> m_output_new_orders_queue;
    CircularBuffer<OrderPtr, QueueSize> m_output_executed_orders_queue;
    CircularBuffer<OrderPtr, QueueSize> m_output_canceled_orders_queue;
    CircularBuffer<std::tuple<OrderPtr, Timestamp>, QueueSize> m_output_replaced_orders_queue;
    CircularBuffer<MDTradePtr, QueueSize> m_output_md_trades_queue;
    OrderExecutionManager m_order_exection_manager;
    std::function<void(OrderPtr)> m_executed_order_callback;
    std::function<void(OrderPtr)> m_canceled_order_callback;
    std::function<void(OrderPtr)> m_replaced_order_callback;
    std::function<void(OrderPtr)> m_new_order_callback;
    std::function<void(MDTradePtr)> m_md_trade_callback;
    Timedelta m_executionLatency{0}, m_marketDataLatency{0};
    Timestamp m_currentTimestamp{0}, m_nextTimestamp{0};
};