#ifndef PMS_MARKETDATA_H
#define PMS_MARKETDATA_H

#include "types.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace pms
{
    // One tick of market data for an instrument
    struct Quote
    {
        InstrumentId id; // e.g., EQ1234
        Price bid{}; // best bid
        Qty bidQty{}; // size at bid
        Price ask{}; // best ask
        Qty askQty{}; // size at ask
        Price last{}; // last traded price
        Timestamp ts{}; // when this quote was observed (not implemented yet)
    };

    // A convenient mark: mid = (bid + ask)/2. Falls back to last if one side is missing
    Price mid(const Quote& q);

    // Push-based market data: subscribe by instrument id, receive Quote updates
    class MarketDataFeed
    {
        public:
            using Handler = std::function<void(const Quote&)>;

            // Register a handler for a specific instrument id
            void subscribe(const InstrumentId& id, Handler h);

            // Push a new quote: updates the internal "last" cache and runs all handlers for that id
            void push(const Quote& q);

            // Access the most recent quote we saw for an instrument (nullptr if none yet)
            const Quote* last(const InstrumentId& id) const;

            // Number of instruments with at last one quote seen
            std::size_t knownInstruments() const;

        private:
            std::unordered_map<InstrumentId, std::vector<Handler>> subs_;
            std::unordered_map<InstrumentId, Quote> last_; // cached latest quote per id
    };
}

#endif