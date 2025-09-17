#ifndef PMS_INSTRUMENT_H
#define PMS_INSTRUMENT_H

#include "types.h"
#include <string>
#include <unordered_map>
#include <istream>

namespace pms
{

    // A financial instrument e.g., equity, futures, fx, crypto, option
    struct instrument
    {
        InstrumentId id; // e.g., AAPL
        std::string symbol; // display symbol (in this case, same as id)
        std::string assetClass;
        std::string currency;
        double tickSize{0.01}; // Minimum price increment
    };

    // An internal database for financial instruments
    class InstrumentRepo
    {
        public:
            // Insert or replace a financial instrument
            void add(const instrument& inst);

            // Retrieve a financial instrument
            const instrument* get(const InstrumentId& id) const;

            // Check if a financial instrument exists
            bool has(const InstrumentId& id) const;

            // Retrieve the size of the repository i.e., number of financial instruments available
            std::size_t size() const;

            // Load instruments from a CSV stream or file
            // - Simple CSV parsing (no quoted fields with embedded commas)
            // - Skips empty lines and lines starting with #
            // - Throws std::runtime_error on malformed data
            void loadFromCSVStream(std::istream& in);
            void loadFromCSVFile(const std::string& path);

        private:
            std::unordered_map<InstrumentId, instrument> map_;
    };

}

#endif