#pragma once

#include "../definitions.h"

namespace CRPT::Core
{
    constexpr double MAXPRICE = std::numeric_limits<double>::max();

    using OrderId = uint64_t;
    using Timestamp = uint64_t;
    using Timedelta = uint64_t;
    using UpdateId = uint64_t;

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

    inline std::string ToString(MarketDataType type)
    {
        switch (type)
        {
        case MarketDataType::Trade:
            return "Trade";
        case MarketDataType::L1Update:
            return "L1Update";
        case MarketDataType::L2Update:
            return "L2Update";
        case MarketDataType::Custom:
            return "Custom";
        case MarketDataType::CustomMultiple:
            return "CustomMultiple";
        }
        return "UnknownMarketDataType";
    }

    inline std::string ToString(OrderType type)
    {
        switch (type)
        {
        case OrderType::Market:
            return "Market";
        case OrderType::Limit:
            return "Limit";
        }
        return "UnknownOrderType";
    }

    inline std::string ToString(OrderState state)
    {
        switch (state)
        {
        case OrderState::Active:
            return "Active";
        case OrderState::Rejected:
            return "Rejected";
        case OrderState::Canceled:
            return "Canceled";
        case OrderState::PendingNew:
            return "PendingNew";
        case OrderState::PendingCancel:
            return "PendingCancel";
        case OrderState::Filled:
            return "Filled";
        case OrderState::PartiallyFilled:
            return "PartiallyFilled";
        }
        return "UnknownOrderState";
    }

    inline std::string ToString(Side side)
    {
        switch (side)
        {
        case Side::Buy:
            return "Buy";
        case Side::Sell:
            return "Sell";
        }
        return "UnknownSide";
    }

    struct Instrument
    {
        std::string InstrumentId;
        std::string Venue;
    };

    using InstrumentPtr = std::string;

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

    using OrderPtr = Order *;

    struct MarketDataUpdate
    {
        UpdateId Id{0};
        Timestamp EventTimestamp{0};
        MarketDataType Type;
    };

    typedef MarketDataUpdate *MarketDataUpdatePtr;

    struct MDTrade : public MarketDataUpdate
    {
        double Price;
        double Qty;
        Timestamp LocalTimestamp;
        Side AggressorSide;
        InstrumentPtr Instrument;

        MDTrade()
        {
            Type = MarketDataType::Trade;
        }

        std::string ToString() const
        {
            std::ostringstream oss;
            oss << "MDTrade { "
                << "Type=" << CRPT::Core::ToString(Type)
                << ", Price=" << Price
                << ", Qty=" << Qty
                << ", AggressorSide=" << CRPT::Core::ToString(AggressorSide)
                << ", EventTimestamp=" << EventTimestamp
                << ", LocalTimestamp=" << LocalTimestamp
                << ", Instrument=" << Instrument
                << " }";
            return oss.str();
        }
    };

    using MDTradePtr = MDTrade *;

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

        std::string ToString() const
        {
            std::ostringstream oss;
            oss << "MDL1Update { "
                << "Type=" << CRPT::Core::ToString(Type)
                << ", AskPrice=" << AskPrice
                << ", BidPrice=" << BidPrice
                << ", AskQty=" << AskQty
                << ", BidQty=" << BidQty
                << ", Qty=" << Qty
                << ", EventTimestamp=" << EventTimestamp
                << ", LocalTimestamp=" << LocalTimestamp
                << ", Instrument=" << Instrument
                << " }";
            return oss.str();
        }
    };

    using MDL1UpdatePtr = MDL1Update *;

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

    using MDL2UpdatePtr = MDL2Update *;

    struct MDCustomUpdate : public MarketDataUpdate
    {
        std::string Text;
        double Payload;
        MDCustomUpdate()
        {
            Type = MarketDataType::Custom;
        }
    };

    using MDCustomUpdatePtr = MDCustomUpdate *;

    struct MDCustomMultipleUpdate : public MarketDataUpdate
    {
        std::string Text;
        std::unordered_map<std::string, double> Payload;
        MDCustomMultipleUpdate()
        {
            Type = MarketDataType::CustomMultiple;
        }
    };

    using MDCustomMultipleUpdatePtr = MDCustomMultipleUpdate *;
}