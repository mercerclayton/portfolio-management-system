#include "marketdata.h"
#include <algorithm>

namespace pms
{
    // Conservative mark: (bid + ask)/2 when both sides exist; otherwise use last; otherwise 0.0
    Price mid(const Quote& q)
    {
        const bool haveBid = q.bid.px > 0.0;
        const bool haveAsk = q.ask.px > 0.0;

        if (haveBid && haveAsk)
        {
            return Price{ (q.bid.px + q.ask.px) / 2.0 };
        }

        if (q.last.px > 0.0)
        {
            return q.last;
        }

        return Price{0.0};
    }

    void MarketDataFeed::subscribe(const InstrumentId& id, Handler h)
    {
        subs_[id].push_back(std::move(h));
    }

    void MarketDataFeed::push(const Quote& q)
    {
        // Cache the latest quote
        last_[q.id] = q;

        // Deliver to subscribers (if any)
        auto it = subs_.find(q.id);
        if (it == subs_.end()) return;

        for (auto& handler : it->second)
        {
            handler(q);
        }
    }

    const Quote* MarketDataFeed::last(const InstrumentId& id) const
    {
        auto it = last_.find(id);
        return (it == last_.end()) ? nullptr : &it->second;
    }

    std::size_t MarketDataFeed::knownInstruments() const
    {
        return last_.size();
    }
}