#pragma once

#include "entity.hpp"
#include "../utils/helpers.hpp"
#include "../definitions.h"

class CSVMarketDataTradesManager
{
public:
    CSVMarketDataTradesManager();
    CSVMarketDataTradesManager(CSVMarketDataTradesManager &&) = delete;
    CSVMarketDataTradesManager(CSVMarketDataTradesManager &) = delete;
    CSVMarketDataTradesManager(const CSVMarketDataTradesManager &) = delete;
    CSVMarketDataTradesManager(std::vector<std::string> paths)
    {
        for (auto &path : paths)
            _loadData(path);
        _sortData();
    }

    auto begin()
    {
        return _data.begin();
    }

    auto end()
    {
        return _data.end();
    }

private:
    void _loadData(const std::string &path)
    {
        auto rawLines = Helpers::ReadCSV(path);
        for (auto &line : rawLines)
        {
            MDTrade update;
            //update->Instrument = InstrumentManager::GetOrCreateInstrument(line[4], line[5]);
            update.EventTimestamp = std::stol(line[0]);
            update.Price = std::stod(line[1]);
            update.Qty = std::stod(line[2]);
            update.AggressorSide = Helpers::ToLower(line[3]) == "buy" ? Side::Buy : Side::Sell;
            _data.push_back(update);
        }
    }

    inline void _sortData()
    {
        // Quite time consuming, but this application is not latency-sensitive
        std::sort(_data.begin(), _data.end(), [](MDTrade &a, MDTrade &b)
                  { return a.EventTimestamp < b.EventTimestamp; });
    }

private:
    std::vector<MDTrade> _data;
    int _cursor{0};
    int _size{0};
};