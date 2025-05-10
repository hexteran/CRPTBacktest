#pragma once

#include "entity.hpp"
#include "../utils/helpers.hpp"
#include "../definitions.h"

namespace CRPT::Core
{
    using namespace CRPT::Utils;

    class IMarketDataTradesManager
    {
    public:
        virtual std::vector<MDTrade>::iterator begin()
        {
            return _data.begin();
        }

        virtual std::vector<MDTrade>::iterator end()
        {
            return _data.end();
        }

    protected:
        std::vector<MDTrade> _data;
    };

    class MDRow
    {
    public:
        using BufferPtr = char *;

        MDRow(const std::vector<MDTrade> &row, const std::string &rowName = "") : m_typeSize(sizeof(MDTrade)),
                                                                                  m_row{BufferPtr(row.data())},
                                                                                  m_rowSize(row.size()),
                                                                                  m_rowName(rowName)
        {
        }

        MDRow(const std::vector<MDL1Update> &row, const std::string &rowName = "") : m_typeSize(sizeof(MDL1Update)),
                                                                                     m_row{BufferPtr(row.data())},
                                                                                     m_rowSize(row.size()),
                                                                                     m_rowName(rowName)
        {
        }

        MDRow(const std::vector<MDCustomUpdate> &row, const std::string &rowName = "") : m_typeSize(sizeof(MDCustomUpdate)),
                                                                                         m_row{BufferPtr(row.data())},
                                                                                         m_rowSize(row.size()),
                                                                                         m_rowName(rowName)
        {
        }

        MDRow(const std::vector<MDCustomMultipleUpdate> &row, const std::string &rowName = "") : m_typeSize(sizeof(MDCustomMultipleUpdate)),
                                                                                                 m_row{BufferPtr(row.data())},
                                                                                                 m_rowSize(row.size()),
                                                                                                 m_rowName(rowName)
        {
        }

        MarketDataUpdatePtr operator[](size_t n) const
        {
            if (n < m_rowSize)
                return (MarketDataUpdatePtr)&m_row[m_typeSize * n];
            else
                return nullptr;
        }

        size_t size()
        {
            return m_rowSize;
        }

    private:
        BufferPtr m_row;
        size_t m_typeSize;
        size_t m_rowSize;
        std::string m_rowName;
    };

    class MarketDataSimulationManager
    {
    public:
        using BufferPtr = char *;
        class iterator
        {
        public:
            iterator(MarketDataSimulationManager &obj) : m_counters(obj.m_buffers.size(), 0),
                                                         m_obj(obj)
            {
                this->operator++();
            }

            bool operator==(const iterator &other)
            {
                return m_currentElement == other.m_currentElement;
                //|| (m_currentElement == nullptr && other.m_currentElement == nullptr);
            }

            bool operator!=(const iterator &other)
            {
                return !(*this == other);
            }

            const MarketDataUpdatePtr &operator*()
            {
                return m_currentElement;
            }

            MarketDataUpdatePtr operator->()
            {
                return m_currentElement;
            }

            iterator operator+(size_t steps)
            {
                ++steps;
                auto counters(m_counters);
                auto currentElement(m_currentElement);
                for (; steps > 0; --steps)
                {
                    int argmin = -1;
                    Timestamp min = std::numeric_limits<Timestamp>::max();
                    for (int i = 0; i < counters.size(); ++i)
                    {
                        if (counters[i] < m_obj.m_buffers[i].size())
                        {
                            MarketDataUpdatePtr update = m_obj.m_buffers[i][counters[i]];
                            if (update->EventTimestamp <= min)
                            {
                                min = update->EventTimestamp;
                                argmin = i;
                            }
                        }
                    }
                    if (argmin == -1)
                    {
                        currentElement = nullptr;
                    }
                    else
                    {
                        currentElement = m_obj.m_buffers[argmin][counters[argmin]]; // dangerous
                        ++counters[argmin];
                    }
                }
                iterator result(m_obj);
                result.m_counters = counters;
                result.m_currentElement = currentElement;
                return result;
            }

            iterator &operator++()
            {
                int argmin = -1;
                Timestamp min = std::numeric_limits<Timestamp>::max();
                for (int i = 0; i < m_counters.size(); ++i)
                {
                    if (m_counters[i] < m_obj.m_buffers[i].size())
                    {
                        MarketDataUpdatePtr update = m_obj.m_buffers[i][m_counters[i]];
                        if (update->EventTimestamp <= min)
                        {
                            min = update->EventTimestamp;
                            argmin = i;
                        }
                    }
                }

                if (argmin == -1)
                {
                    m_currentElement = nullptr;
                }
                else
                {
                    m_currentElement = m_obj.m_buffers[argmin][m_counters[argmin]]; // dangerous
                    ++m_counters[argmin];
                }
                return *this;
            };

        private:
            friend MarketDataSimulationManager;
            std::vector<u_int64_t> m_counters;
            MarketDataUpdatePtr m_currentElement{nullptr};
            MarketDataSimulationManager &m_obj;
        };

        MarketDataSimulationManager() = default;
        MarketDataSimulationManager(std::vector<MDRow> buffers) : m_buffers{buffers}
        {
        }

        iterator begin()
        {
            return iterator(*this);
        }

        iterator end()
        {
            iterator iter(*this);
            iter.m_currentElement = nullptr;

            return iter;
        }

        void AddRow(const MDRow &row)
        {
            m_buffers.push_back(row);
        }

        void Clear()
        {
            m_buffers.clear();
        }

    private:
        std::vector<MDRow> m_buffers;
    };

    class CSVMarketDataTradesManager : public IMarketDataTradesManager
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

        std::vector<MDTrade> &GetTrades()
        {
            return _data;
        }

    private:
        void _loadData(const std::string &path)
        {
            auto rawLines = Helpers::ReadCSV(path);
            for (auto &line : rawLines)
            {
                MDTrade update;
                // update->Instrument = InstrumentManager::GetOrCreateInstrument(line[4], line[5]);
                update.EventTimestamp = std::stol(line[0]);
                update.Price = std::stod(line[1]);
                update.Qty = std::stod(line[2]);
                update.AggressorSide = Helpers::ToLower(line[3]) == "buy" ? Side::Buy : Side::Sell;
                update.Instrument = line[4];
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
        int _cursor{0};
        int _size{0};
    };
}