#pragma once

#include "../definitions.h"

class Helpers
{
public:
    static inline u_int64_t GetTimeNow()
    {
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    }

    static std::string TimestampToStr(u_int64_t ts, std::string format = "%Y-%m-%d %H:%M:%S")
    {
        auto now = std::chrono::system_clock::now();
        auto onems = std::chrono::microseconds(1);
        u_int64_t epochus = ts;
        time_t epoch = epochus / 1000000000;
        struct tm tms
        {
        };
        localtime_r(&epoch, &tms);
        char buf[128];
        size_t nb = strftime(buf, sizeof(buf), format.c_str(), &tms);
        nb += ::sprintf(&buf[nb], ".%06d", int(epochus % 1000000000));
        return std::string(buf, nb);
    }

    static std::string GetRandomString(size_t length)
    {
        auto randchar = []() -> char
        {
            const char charset[] =
                "0123456789"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";
            const size_t max_index = (sizeof(charset) - 1);
            return charset[rand() % max_index];
        };
        std::string str(length, 0);
        std::generate_n(str.begin(), length, randchar);
        return str;
    }

    static std::string ToLower(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(), [](char c)
                       { return std::tolower(c); });
        return str;
    }

    static double Round(double value, int digits)
    {
        auto d = std::pow(10, digits);
        return std::round(value / 0.0001) * 0.0001;
    }

    static std::vector<std::vector<std::string>> ReadCSV(const std::string &path, char sep = ',')
    {
        std::vector<std::vector<std::string>> result;
        std::ifstream file{path};
        std::string line;
        while (std::getline(file, line, '\n'))
        {
            std::vector<std::string> elems;
            std::stringstream ss{line};
            std::string elem;
            while (std::getline(ss, elem, ','))
                elems.push_back(elem);
            result.push_back(elems);
        }
        file.close();
        return result;
    }

    static void WriteCSV(const std::string &path, const std::vector<std::vector<std::string>> &data, char sep = ',')
    {
        std::ofstream file{path};
        for (auto &line : data)
        {
            std::string l{};
            for (auto &elem : line)
            {
                l += elem + sep;
            }
            l[l.size() - 1] = '\n';
            file << l;
        }
        file.close();
    }
};