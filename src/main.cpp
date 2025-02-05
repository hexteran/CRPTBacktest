#include "core/simulation.hpp"

static std::vector<Order> Orders{1000000, Order()};
static u_int32_t OrderCounter;
static OrderPtr SendOrder(InstrumentPtr instrument, double price, double qty, Side side, OrderType type);

class SMA
{
private:
    double _sum;
    int _period;
    std::queue<double> _window;

public:
    SMA(int period) : _sum(0.0), _period(period)
    {
        if (period <= 0)
            throw std::invalid_argument("Period must be a positive integer.");
    }

    double GetValue() const
    {
        if (_window.empty())
            return 0.0;
        return _sum / _window.size();
    }
    double Push(double price)
    {
        _window.push(price);
        _sum += price;

        if (static_cast<int>(_window.size()) > _period) {
            _sum -= _window.front();
            _window.pop();
        }

        return GetValue();
    }
};

class Strategy
{
public:
    static void Run()
    {
    }

    static void SendMarketOrder(InstrumentPtr instrument, Side side, double qty)
    {   
        SendOrder(instrument, 0, qty, side, OrderType::Market);
    }

    void OnTrade(MDTradePtr trade)
    {
        _smaShort.Push(trade->Price);
        _smaLong.Push(trade->Price);
        ++_numTrades;
        if (_numTrades < 1200)
            return;

        if (_smaLong.GetValue() > _smaShort.GetValue() && !_shortPosition)
        {
            //std::cout << trade->EventTimestamp << " " <<  trade->Price << '\n';
            _shortPosition = true;
            SendMarketOrder(trade->Instrument, Side::Sell, 1);
        }
        else if (_smaLong.GetValue() < _smaShort.GetValue() && _shortPosition)
        {
            //std::cout << trade->EventTimestamp << " " <<  trade->Price << '\n';
            _shortPosition = false;
            SendMarketOrder(trade->Instrument, Side::Buy, 1);
        }
    }

    void OnL1Update(MDL1UpdatePtr Order)
    {
    }

    void OnNewOrder(OrderPtr Order)
    {   
        
    }

    void OnOrderFilled(OrderPtr Order)
    {
        //std::cout << Order->ToString() << '\n';
        
    }

    void OnOrderCanceled(OrderPtr Order)
    {
    }

    void OnOrderRejected(OrderPtr Order)
    {
    }

private:
    SMA _smaShort{600};
    SMA _smaLong{1200};
    bool _shortPosition{false};
    int _numTrades{0};
    std::vector<Order> _orders{100000, Order()};
    u_int32_t _orderCount{0};
} StrategyLogic;

void executed_order_callback(OrderPtr order)
{
    StrategyLogic.OnOrderFilled(order);
}

void canceled_order_callback(OrderPtr order)
{
    StrategyLogic.OnOrderCanceled(order);
}

void new_order_callback(OrderPtr order)
{
    StrategyLogic.OnNewOrder(order);
}

void replace_order_callback(OrderPtr order)
{
    StrategyLogic.OnNewOrder(order);
}

void md_trade_callback(MDTradePtr trade)
{
    StrategyLogic.OnTrade(trade);
}

static CSVMarketDataTradesManager Manager({"/home/hexteran/data/ASTSUSDT_BINANCE.csv"});
static Simulation<10000> StrategySimulation(Manager, 0, 0, 
    executed_order_callback,
    canceled_order_callback,
    replace_order_callback,
    new_order_callback,
    md_trade_callback);

static OrderPtr SendOrder(InstrumentPtr instrument, double price, double qty, Side side, OrderType type)
{
    auto& newOrder = Orders[OrderCounter];
    newOrder.Type = type;
    newOrder.Price = price;
    newOrder.Qty = qty;
    newOrder.OrderSide = side;
    ++OrderCounter;
    StrategySimulation.OnNewOrder(&newOrder);
    return &newOrder;
}

int main()
{
    StrategySimulation.Run();
    return 0;
}