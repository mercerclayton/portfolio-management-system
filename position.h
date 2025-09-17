#ifndef PMS_POSITION_HPP
#define PMS_POSITION_HPP

#include "types.h"
#include <deque>
#include <string>

namespace pms
{
    // An executed lot that is currently open
    struct PositionLot {
        Side side;       // Buy = long lot, Sell = short lot
        Qty qty{};        // remaining open quantity in this lot
        Price entryPx{};    // price at which this lot was opened
        Timestamp ts{};   // Not implemented yet
    };

    // A position tracked per instrument
    // Stores open lots (FIFO), net quantity, avg entry price, and realized PnL
    struct Position {
        InstrumentId instrument;
        std::string  currency{"USD"};

        // Snapshot values kept in sync after each fill
        double netQty{0.0};      // long > 0, short < 0
        Price  avgPx{0.0};       // volume-weighted entry px of open side
        Money  realized{currency, 0.0}; // realized PnL to date (same currency)

        std::deque<PositionLot> lots; // FIFO queue of open lots

        // Apply a fill (trade) to this position and update realized/avg/net lots
        void onFill(Side side, Qty fillQty, Price fillPx, Timestamp ts = Timestamp{});

        // Mark-to-market PnL on the currently open position (does not change state)
        Money unrealized(Price mark) const;

    private:
        // Recompute netQty and avgPx from current lots
        void recomputeAverages_();
    };
}

#endif
