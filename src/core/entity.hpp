#pragma once

#include "../definitions.h"

constexpr double MAXPRICE = std::numeric_limits<double>::max();

using OrderId = uint64_t;
using Timestamp = uint64_t;
using Timedelta = uint64_t;

enum class MarketDataType
{
    Trade,
    L1Update,
    L2Update,
    Custom,
    CustomMultiple
};

enum class OrderType
{
    Market,
    Limit
};

enum class OrderState
{
    Active,
    Rejected,
    Canceled,
    PendingNew,
    PendingCancel,
    Filled,
    PartiallyFilled
};

enum class Side : u_int8_t
{
    Buy = 0,
    Sell = 1
};

struct Instrument
{
    std::string InstrumentId;
    std::string Venue;
};

using InstrumentPtr = std::string;

struct ExecutionReport
{
    OrderId Id;
    std::string ClOrdId;
    OrderState State = OrderState::PendingNew;
    InstrumentPtr Instrument;
    std::string Error;
    double Price = 0;
    double Qty;
    double LastExecutedQty = 0;
    double FilledQty = 0;
    double LastExecutedPrice = 0;
    Timestamp EventTimestamp = 0;
    Timestamp LocalTimestamp = 0;
};

using ExecutionReportPtr = ExecutionReport*;

struct Order
{
    OrderId Id;
    std::string ClOrdId;
    OrderState State = OrderState::PendingNew;
    OrderType Type;
    Side OrderSide;
    InstrumentPtr Instrument;
    std::string Text = "";
    std::string StrategyId = "";
    double Price = 0;
    double Qty;
    double FilledQty = 0;
    double LastExecPrice = 0;
    Timestamp CreateTimestamp = 0;
    Timestamp LastReportTimestamp = 0;

    std::string ToString() const
    {
        std::ostringstream oss;
        oss << "Order { "
            << "Id=" << Id
            << ", ClOrdId=\"" << ClOrdId << "\""
            << ", Instrument=" << Instrument
            << ", Text=\"" << Text << "\""
            << ", StrategyId=\"" << StrategyId << "\""
            << ", Price=" << Price
            << ", Qty=" << Qty
            << ", FilledQty=" << FilledQty
            << ", LastExecPrice=" << LastExecPrice
            << ", CreateTimestamp=" << CreateTimestamp
            << ", LastReportTimestamp=" << LastReportTimestamp
            << " }";
        return oss.str();
    }
};

using OrderPtr = Order*;

struct MarketDataUpdate
{
    MarketDataType Type;
    Timestamp EventTimestamp;
};

typedef MarketDataUpdate* MarketDataUpdatePtr;

struct MDTrade : public MarketDataUpdate
{
    double Price;
    double Qty;
    Side AggressorSide;
    Timestamp LocalTimestamp;
    InstrumentPtr Instrument;

    MDTrade()
    {
        Type = MarketDataType::Trade;
    }
};

using MDTradePtr = MDTrade*;

struct MDL1Update : public MarketDataUpdate
{
    double AskPrice;
    double BidPrice;
    double AskQty;
    double BidQty;
    double Qty;
    Timestamp LocalTimestamp;
    InstrumentPtr Instrument;

    MDL1Update()
    {
        Type = MarketDataType::L1Update;
    }
};

using MDL1UpdatePtr = MDL1Update*;

struct MDL2Level
{
    double Price;
    double Qty;
};

struct MDL2Update : public MarketDataUpdate
{
    std::vector<MDL2Level> Ask;
    std::vector<MDL2Level> Bid;
    Timestamp LocalTimestamp;
    InstrumentPtr Instrument;
    MDL2Update()
    {
        Type = MarketDataType::L2Update;
    }
};

using MDL2UpdatePtr = MDL2Update*;

struct MDCustomUpdate : public MarketDataUpdate
{
    std::string Text;
    double Payload;
    MDCustomUpdate()
    {
        Type = MarketDataType::Custom;
    }
};

using MDCustomUpdatePtr = MDCustomUpdate*;

struct MDCustomMultipleUpdate : public MarketDataUpdate
{
    std::string Text;
    std::unordered_map<std::string, double> Payload;
    MDCustomMultipleUpdate()
    {
        Type = MarketDataType::CustomMultiple;
    }
};

using MDCustomMultipleUpdatePtr = MDCustomMultipleUpdate*;