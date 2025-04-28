#include "../convenience/clickhouse.hpp"
#include "../core/simulation.hpp"

class PnDQuoter
{
public:
    PnDQuoter()
    : orders(500000, Order()),
      m_simulation(m_md_manager, 0, 0,
                     [this](OrderPtr order) { this->OnOrderFilled(order); },
                     [this](OrderPtr order) { this->OnOrderCanceled(order); },
                     [this](OrderPtr order) { this->OnOrderReplaced(order); },
                     [this](OrderPtr order) { this->OnNewOrder(order); },
                     [this](MDTradePtr trade) { this->OnMDTrade(trade); },
                     [this](MDL1UpdatePtr trade) { this->OnL1Update(trade); },
                     [this](MDCustomUpdatePtr update) { this->OnMDCustomUpdate(update); },
                     [this](MDCustomMultipleUpdatePtr update) { this->OnMDCustomMultipleUpdate(update); }
                     )
    {

    }
    
    void OnOrderCanceled(OrderPtr order){}

    void OnOrderReplaced(OrderPtr order){}

    void OnNewOrder(OrderPtr order){}

    void OnMDCustomUpdate(MDCustomUpdatePtr update){}

    void OnL1Update(MDL1UpdatePtr update){}

    void OnOrderFilled(OrderPtr order)
    {
        std::cout << "Order filled: " << Helpers::TimestampToStr(order->CreateTimestamp) << ';' << Helpers::TimestampToStr(order->LastReportTimestamp)
            << ',' << order->LastExecPrice
            << '\n';

    }

    void SendQuotes(int n_quotes, double price, double step)
    {
        for(int i = 1; i <= n_quotes; ++i)
        {
            sendOrder("THEUSDT", price + i*step, 1, Side::Sell, OrderType::Limit);
        }
    }

    void OnMDTrade(MDTradePtr trade)
    {
        //std::cout << trade->Price << ' ' << m_prev_price << '\n';
        if (m_prev_price != -1)
        {
            if ((trade->Price/m_prev_price) - 1 > 0.05 && !m_sent)
            {
                std::cout << trade->Price << ' ' << m_prev_price << '\n';
                std::cout << "Sending Orders: " << Helpers::TimestampToStr(trade->EventTimestamp) << '\n';
                SendQuotes(10, trade->Price, 0.1*m_prev_price/10);
                m_sent = true;
            }
        }  
        m_prev_price = trade->Price;
        m_prev_ts = trade->LocalTimestamp;
    }

    void OnMDCustomMultipleUpdate(MDCustomMultipleUpdatePtr custom_update)
    {
    }
    
    void Run(const std::vector<MDTrade>& trades, const std::string& symbol)
    {
        m_md_manager.AddRow(MDRow{trades, symbol});
        m_simulation.Run();
        m_md_manager.Clear();
    }

private:
    OrderPtr sendOrder(const std::string &instrument,
                   double price, 
                   double qty,
                   Side side, 
                   OrderType type, 
                   const std::string &text = "")
    {
        orders[m_order_coursor].Instrument = instrument;
        orders[m_order_coursor].Price = price;
        orders[m_order_coursor].Qty = qty;
        orders[m_order_coursor].OrderSide = side;
        orders[m_order_coursor].Type = type;
        orders[m_order_coursor].Text = text;
        m_simulation.OnNewOrder(&orders[m_order_coursor]);
        return &orders[m_order_coursor++];
    }

    OrderPtr cancelOrder(OrderPtr order)
    {
        m_simulation.OnCancelOrder(order);
        return order;
    }

private:
    MarketDataSimulationManager m_md_manager;
    Simulation<10000> m_simulation;
    std::vector<Order> orders;
    uint32_t m_order_coursor{0};

    Timestamp m_prev_ts{0};
    double m_prev_price{-1};
    bool m_sent{false};
};

int main()
{
    auto data = ClickhouseMarketDataFetcher<MDTrade>::Fetch(
        "localhost", 19000, 
        "default", "root",
        "SELECT * FROM binance_futures_usdt_trades WHERE event_timestamp > '2025-02-14 02:04:26' and event_timestamp < '2025-02-14 02:24:26' and symbol == 'THEUSDT'");
    std::sort(data.begin(), data.end(), [](MDTrade& trade_a, MDTrade& trade_b){return trade_a.EventTimestamp < trade_b.EventTimestamp;});
    std::cout << Helpers::TimestampToStr(data[0].EventTimestamp) << "\nSTARTING ... \n";
    PnDQuoter strategy;
    strategy.Run(data, "RAREUSDT");
    return 0;
}