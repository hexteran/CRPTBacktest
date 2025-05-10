#pragma once
#include "../definitions.h"
#include "entity.hpp"

namespace CRPT::Core
{
    using namespace CRPT::Utils;

    class OrderExecutionManager
    {
    public:
        void AddNewOrder(OrderPtr order)
        {
            if (order->Type == OrderType::Limit)
            {
                if ((order->OrderSide == Side::Buy && m_lastSellMarketPrice < order->Price) ||
                    (order->OrderSide == Side::Sell && m_lastBuyMarketPrice > order->Price))
                    order->Type = OrderType::Market;
            }
            order->OrderSide == Side::Buy ? m_bid.push(order) : m_ask.push(order);
            if (order->Type == OrderType::Market)
                order->Price = order->OrderSide == Side::Buy ? MAXPRICE : 0.;
        }

        std::vector<OrderPtr> MatchWithPrice(double price, Side side)
        {
            // Always filling our orders in full, no partial fills
            std::vector<OrderPtr> result;
            if (side == Side::Sell)
            {
                m_lastSellMarketPrice = price;
                while (!m_bid.empty() && (m_bid.top()->Price >= price || m_bid.top()->Type == OrderType::Market))
                {
                    result.push_back(m_bid.top());
                    m_bid.pop();
                    result.back()->FilledQty = result.back()->Qty;
                    if (result.back()->Type == OrderType::Market)
                        result.back()->LastExecPrice = price;
                    else
                        result.back()->LastExecPrice = result.back()->Price;
                }
            }
            else if (side == Side::Buy)
            {
                m_lastBuyMarketPrice = price;
                while (!m_ask.empty() && (m_ask.top()->Price <= price || m_ask.top()->Type == OrderType::Market))
                {
                    result.push_back(m_ask.top());
                    m_ask.pop();
                    result.back()->FilledQty = result.back()->Qty;
                    if (result.back()->Type == OrderType::Market)
                        result.back()->LastExecPrice = price;
                    else
                        result.back()->LastExecPrice = result.back()->Price;
                }
            }
            return result;
        }

        void ReplaceOrder(OrderPtr order, double price, double qty)
        {
            if (order->OrderSide == Side::Buy)
            {
                std::vector<OrderPtr> buffer;
                while (!m_bid.empty())
                {
                    buffer.push_back(m_bid.top());
                    m_bid.pop();
                }
                while (!buffer.empty())
                {
                    m_bid.push(buffer.back());
                    buffer.pop_back();
                }
            }
            else
            {
                std::vector<OrderPtr> buffer;
                while (!m_ask.empty())
                {
                    buffer.push_back(m_ask.top());
                    m_ask.pop();
                }
                while (!buffer.empty())
                {
                    m_ask.push(buffer.back());
                    buffer.pop_back();
                }
            }
        }

        void CancelOrder(OrderPtr order)
        {
            if (order->OrderSide == Side::Buy)
            {
                std::vector<OrderPtr> buffer;
                while (!m_bid.empty())
                {
                    if (m_bid.top() != order)
                        buffer.push_back(m_bid.top());
                    m_bid.pop();
                }
                while (!buffer.empty())
                {
                    m_bid.push(buffer.back());
                    buffer.pop_back();
                }
            }
            else
            {
                std::vector<OrderPtr> buffer;
                while (!m_ask.empty())
                {
                    if (m_ask.top() != order)
                        buffer.push_back(m_ask.top());
                    m_ask.pop();
                }
                while (!buffer.empty())
                {
                    m_ask.push(buffer.back());
                    buffer.pop_back();
                }
            }
        }

    private:
        struct AskComparator
        {
            bool operator()(const OrderPtr &a, const OrderPtr &b) const
            {
                return a->Price > b->Price;
            }
        };

        struct BidComparator
        {
            bool operator()(const OrderPtr &a, const OrderPtr &b) const
            {
                return a->Price < b->Price;
            }
        };

        std::priority_queue<OrderPtr, std::vector<OrderPtr>, AskComparator> m_ask;
        std::priority_queue<OrderPtr, std::vector<OrderPtr>, BidComparator> m_bid;

        double m_lastBuyMarketPrice{0};
        double m_lastSellMarketPrice{MAXPRICE};
    };
}