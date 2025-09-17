#include "instrument.h"
#include <fstream>
#include <vector>
#include <stdexcept>
#include <algorithm>

namespace pms
{
    // Helper functions
    // Removes leading and trailing whitespaces from a string
    static std::string trim(std::string s)
    {
        auto not_space = [](unsigned char c) { return !std::isspace(c); };
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
        // .base() returns a forward iterator that is [reverse_iterator + 1]
        s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
        return s;
    }

    // CSV splitter which splits on commas; does not support quoted commas (quotation marks)
    static std::vector<std::string> splitCSVLine(const std::string& line)
    {
        std::vector<std::string> out;
        std::string cur;
        for (const char c : line)
        {
            if (c == ',')
            {
                out.push_back(trim(cur));
                cur.clear();
            } else
            {
                cur.push_back(c); // .push_back() appends a single character
            }
        }
        out.push_back(trim(cur));
        return out;
    }

    // Inserts/replaces a financial instrument in the repository based on id
    void InstrumentRepo::add(const instrument& inst)
    {
        map_[inst.id] = inst;
    }

    // Retrieves a financial instrument based on id
    // Returns a null pointer if not found
    const instrument* InstrumentRepo::get(const std::string& id) const
    {
        const auto it = map_.find(id);
        return (it == map_.end()) ? nullptr : &it->second;
    }

    bool InstrumentRepo::has(const InstrumentId& id) const
    {
        return map_.contains(id);
    }

    std::size_t InstrumentRepo::size() const
    {
        return map_.size();
    }

    void InstrumentRepo::loadFromCSVStream(std::istream& in)
    {
        std::string line;

        // Header required. Function returns quietly for empty stream
        if (!std::getline(in, line)) return;
        auto headers = splitCSVLine(line);
        if (headers.size() < 5)
        {
            throw std::runtime_error("Instrument CSV: need header 'id,symbol,assetClass,currency,tickSize'");
        }

        while (std::getline(in, line))
        {
            // Comments denoted by #
            auto t = trim(line);
            if (t.empty() || t[0] == '#') continue;

            auto cols = splitCSVLine(t);
            if (cols.size() < 5)
            {
                throw std::runtime_error("Instrument CSV: bad row: " + t);
            }

            instrument inst;
            inst.id = cols[0];
            inst.symbol = cols[1];
            inst.assetClass = cols[2];
            inst.currency = cols[3];
            try
            {
                inst.tickSize = std::stod(cols[4]);
            } catch (...)
            {
                throw std::runtime_error("Instrument CSV: invalid tickSize: " + cols[4]);
            }
            add(inst);
        }
    }

    void InstrumentRepo::loadFromCSVFile(const std::string& path)
    {
        std::ifstream in(path);
        if (!in)
        {
            throw std::runtime_error("Cannot open instruments file: " + path);
        }
        loadFromCSVStream(in);
    }

}