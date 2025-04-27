#pragma once

#include <gtest/gtest.h>

#include "../src/convenience/clickhouse.hpp"

TEST(ClickhouseMarketDataFetcherTest, FetchTrades)
{
    auto result = ClickhouseMarketDataFetcher<MDTrade>::Fetch("localhost", 19000, "default", "root",
        "SELECT * FROM binance_spot_usdt_trades WHERE symbol = 'RAREUSDT' ORDER BY event_timestamp");
    EXPECT_EQ(result.size(), 3333385);
}