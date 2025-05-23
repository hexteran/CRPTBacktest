#pragma once

#include "../core/market_data_simulation_manager.hpp"

#include <clickhouse/client.h>

namespace CRPT::Convenience
{
    using namespace CRPT::Core;
    template <class T>
    concept IsMarketDataUpdate = std::derived_from<T, MarketDataUpdate>;

    template <IsMarketDataUpdate T>
    class ClickhouseMarketDataFetcher
    {
    public:
        static std::vector<T> Fetch(
            const std::string &host,
            unsigned int port,
            const std::string &user,
            const std::string &password,
            const std::string &query)
        {
            clickhouse::ClientOptions opts;
            opts.SetHost(host)
                .SetPort(port)
                .SetUser(user)
                .SetPassword(password)
                .SetDefaultDatabase("default");

            std::vector<T> result;
            clickhouse::Client client(opts);
            client.Select(query,
                          [&result](const clickhouse::Block &block)
                          {
                              m_process(block, result);
                          });

            return result;
        }

    private:
        template <IsMarketDataUpdate F>
        static void m_process(const clickhouse::Block &block, std::vector<F> &result)
        {
        }
    };

    // GCC doesn't allow method specializations within the class body
    template <>
    template <>
    void ClickhouseMarketDataFetcher<MDTrade>::m_process<MDTrade>(const clickhouse::Block &block, std::vector<MDTrade> &result)
    {
        using namespace clickhouse;

        if (block.GetRowCount() == 0)
            return;

        auto col_id = block[0]->As<ColumnUInt64>();
        auto col_symbol = block[1]->As<ColumnLowCardinalityT<ColumnString>>();
        auto col_price = block[2]->As<ColumnFloat32>();
        auto col_qty = block[3]->As<ColumnFloat32>();
        auto col_side = block[4]->As<ColumnEnum8>();
        auto col_ts = block[5]->As<ColumnDateTime64>();

        for (size_t i = 0; i < block.GetRowCount(); ++i)
        {
            MDTrade md;
            md.Id = col_id->At(1);
            md.Instrument = col_symbol->At(i);
            md.Price = static_cast<double>(col_price->At(i));
            md.Qty = static_cast<double>(col_qty->At(i));
            md.AggressorSide = static_cast<Side>(col_side->At(i));
            Timestamp ns = col_ts->At(i);
            md.EventTimestamp = ns;
            md.LocalTimestamp = ns;
            result.emplace_back(std::move(md));
        }
    }

    template <>
    template <>
    void ClickhouseMarketDataFetcher<MDL1Update>::m_process<MDL1Update>(const clickhouse::Block &block, std::vector<MDL1Update> &result)
    {
    }
}