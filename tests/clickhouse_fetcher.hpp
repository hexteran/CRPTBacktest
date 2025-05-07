#pragma once

#include <gtest/gtest.h>

#include "../src/convenience/clickhouse.hpp"

TEST(ClickhouseMarketDataFetcherTest, FetchTrades)
{
    auto result = ClickhouseMarketDataFetcher<MDTrade>::Fetch("localhost", 19000, "default", "root",
        "SELECT * FROM binance_futures_um_trades WHERE symbol = 'RAREUSDT'");
}