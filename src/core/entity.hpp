#pragma once

#include "../definitions.h"

constexpr double MAXPRICE = std::numeric_limits<double>::max();

typedef u_int64_t OrderId;
typedef u_int64_t Timestamp;
typedef u_int64_t Timedelta;

enum class MarketDataType
{
    Trade,
    L1Update,
    L2Update
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
typedef Instrument *InstrumentPtr;

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

typedef ExecutionReport *ExecutionReportPtr;

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
    std::vector<ExecutionReportPtr> Reports;

    std::string ToString() const
    {
        std::ostringstream oss;
        oss << "Order { "
            << "Id=" << Id
            << ", ClOrdId=\"" << ClOrdId << "\""
            << ", Instrument=" << (Instrument ? Instrument->InstrumentId + "@" + Instrument->Venue : "null")
            << ", Text=\"" << Text << "\""
            << ", StrategyId=\"" << StrategyId << "\""
            << ", Price=" << Price
            << ", Qty=" << Qty
            << ", FilledQty=" << FilledQty
            << ", LastExecPrice=" << LastExecPrice
            << ", CreateTimestamp=" << CreateTimestamp
            << ", LastReportTimestamp=" << LastReportTimestamp
            << ", ReportsCount=" << Reports.size()
            << " }";
        return oss.str();
    }
};

typedef Order *OrderPtr;

struct MarketDataUpdate
{
    MarketDataType Type;
};

typedef std::shared_ptr<MarketDataUpdate> MarketDataUpdatePtr;

struct MDTrade : public MarketDataUpdate
{
    double Price;
    double Qty;
    Side AggressorSide;
    Timestamp TradeTimestamp;
    Timestamp EventTimestamp;
    Timestamp LocalTimestamp;
    InstrumentPtr Instrument;

    MDTrade()
    {
        Type = MarketDataType::Trade;
    }
};

typedef MDTrade *MDTradePtr;

struct MDL1Update : public MarketDataUpdate
{
    double AskPrice;
    double BidPrice;
    double AskQty;
    double BidQty;
    double Qty;
    Timestamp EventTimestamp;
    Timestamp LocalTimestamp;
    InstrumentPtr Instrument;

    MDL1Update()
    {
        Type = MarketDataType::L1Update;
    }
};

typedef MDL1Update *MDL1UpdatePtr;

struct MDL2Level
{
    double Price;
    double Qty;
};

struct MDL2Update : public MarketDataUpdate
{
    std::vector<MDL2Level> Ask;
    std::vector<MDL2Level> Bid;
    Timestamp EventTimestamp;
    Timestamp LocalTimestamp;
    InstrumentPtr Instrument;
    MDL2Update()
    {
        Type = MarketDataType::L2Update;
    }
};

typedef MDL2Update *MDL2UpdatePtr;