//#include "../definitions.h"
#include "../core/simulation.hpp"

class MarketMaking
{
public:
    MarketMaking() = default;
    MarketMaking(std::string ttf_midprices, std::string the_midprices, std::string the_trades)
    : orders(500000, Order()),
      m_simulation(m_md_manager, 0, 0,
                     [this](OrderPtr order) { this->OnOrderFilled(order); },
                     [this](OrderPtr order) { this->OnOrderCanceled(order); },
                     [this](OrderPtr order) { this->OnOrderReplaced(order); },
                     [this](OrderPtr order) { this->OnNewOrder(order); },
                     [this](MDTradePtr trade) { this->OnMDTrade(trade); },
                     [this](MDCustomUpdatePtr update) { this->OnMDCustomUpdate(update); },
                     [this](MDCustomMultipleUpdatePtr update) { this->OnMDCustomMultipleUpdate(update); }
                     )
    {
        readCSV(ttf_midprices, m_ttf_midprices, "ttf");
        readCSV(the_midprices, m_the_midprices, "the");
        readCSV(the_trades, m_the_trades);
    }
    
    void OnOrderCanceled(OrderPtr order){}

    void OnOrderReplaced(OrderPtr order){}

    void OnNewOrder(OrderPtr order){}

    void OnMDCustomUpdate(MDCustomUpdatePtr update){}

    void OnOrderFilled(OrderPtr order)
    {

    }

    void OnMDTrade(MDTradePtr trade)
    {

    }


    void OnMDCustomMultipleUpdate(MDCustomMultipleUpdatePtr custom_update)
    {
        if (custom_update->Text == "ttf")
        {
            if (custom_update->Payload["midprice"] < 0.01 || custom_update->Payload["spread"] > 0.3)
                RemoveQuotes();
                
            else if (std::abs(custom_update->Payload["midprice"] - last_ref_price) > Sensitivity)
            {
                last_ref_price = custom_update->Payload["midprice"];
                ReplaceQuotes();
            }
        }       
        else
        {
            if (custom_update->Payload["midprice"] > 0.01 || custom_update->Payload["spread"] < 0.3)
            {
                Offset = custom_update->Payload["midprice"] - last_ref_price;
                last_quoted_midprice = custom_update->Payload["midprice"];
            }
        }
    }

    void RemoveQuotes()
    {
        if (ask_order != nullptr)
            cancelOrder(ask_order);
        if (bid_order != nullptr)
            cancelOrder(bid_order);
    }

    void ReplaceQuotes()
    {
        RemoveQuotes();
        if (Offset == 0)
            return;
        bid_order = sendOrder("THE", last_ref_price + Offset - last_ref_price*Spread/2, 5, Side::Buy, OrderType::Limit);
        ask_order = sendOrder("THE", last_ref_price + Offset + last_ref_price*Spread/2, 5, Side::Sell, OrderType::Limit);
    }

    void Run()
    {
        m_md_manager.AddRow(MDRow{m_the_trades, "THE"});
        m_md_manager.AddRow(MDRow{m_the_midprices, "THE_midprice"});
        m_md_manager.AddRow(MDRow{m_ttf_midprices, "TTF_midprice"});

        m_simulation.Run();

        m_md_manager.Clear();
    }


private:
    void readCSV(std::string path, std::vector<MDCustomMultipleUpdate>& out, std::string text = "")
    {
        auto buffer = Helpers::ReadCSV(path, ',');
        for (int i = 1; i < buffer.size(); ++i)
        {
            MDCustomMultipleUpdate update;
            update.Text = text;
            update.EventTimestamp = std::stol(buffer[i][0]);
            update.Payload["midprice"] = std::stod(buffer[i][1]);
            update.Payload["spread"] = std::stod(buffer[i][2]);
            out.push_back(update);
        }
    }  

    void readCSV(std::string path, std::vector<MDTrade>& out)
    {
        auto buffer = Helpers::ReadCSV(path, ',');
        for (int i = 1; i < buffer.size(); ++i)
        {
            MDTrade trade_buy;
            trade_buy.AggressorSide = Side::Buy;
            trade_buy.EventTimestamp = std::stol(buffer[i][0]);
            trade_buy.Price = std::stod(buffer[i][1]);
            trade_buy.Qty = std::stod(buffer[i][2]);
            trade_buy.Instrument = "THE";
            MDTrade trade_sell = trade_buy;
            trade_sell.AggressorSide = Side::Sell;
            m_the_trades.push_back(trade_buy);
            m_the_trades.push_back(trade_sell);
        }
    }

    OrderPtr sendOrder(const std::string &instrument,
                   double price, 
                   double qty,
                   Side side, 
                   OrderType type, 
                   const std::string &text = "")
    {
        orders[order_coursor].Instrument = instrument;
        orders[order_coursor].Price = price;
        orders[order_coursor].Qty = qty;
        orders[order_coursor].OrderSide = side;
        orders[order_coursor].Type = type;
        orders[order_coursor].Text = text;
        m_simulation.OnNewOrder(&orders[order_coursor]);
        return &orders[order_coursor++];
    }

    OrderPtr cancelOrder(OrderPtr order)
    {
        m_simulation.OnCancelOrder(order);
        return order;
    }

private:
    std::vector<MDCustomMultipleUpdate> m_ttf_midprices, m_the_midprices;
    std::vector<MDTrade> m_the_trades;

    MarketDataSimulationManager m_md_manager;
    Simulation<10000> m_simulation;

    std::vector<Order> orders;
    int order_coursor = 0;
    int counter = 0;
    OrderPtr ask_order{nullptr}, bid_order{nullptr};

    double Offset = 1;
    double Sensitivity = 0.05;
    double Spread = 0.01;
    double last_ref_price = 0, last_quoted_midprice = 0;//*/
};

int main()
{
    MarketMaking mm("/home/hexteran/temp/data/ttf.csv", "/home/hexteran/temp/data/the.csv", "/home/hexteran/temp/data/the_trades.csv" );
    std::cout << "Loaded\n";
    
    auto start = std::chrono::high_resolution_clock::now();

    mm.Run();
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "mm.Run() took " << duration.count() << " milliseconds." << std::endl;
    
    return 0;
}