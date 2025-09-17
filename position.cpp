#include "position.h"
#include <algorithm>
#include <cassert>

namespace pms
{
    // Helper function
    // Match against the opposite-side lots (FIFO) and realize PnL
    static void closeAgainstFIFOLots(std::deque<PositionLot>& lots, Side incomingSide, Qty& incomingQty,
        Price incomingPx, Money& realized) {
        // Opposite side to close
        // if we Buy, we close Sell lots; if we Sell, we close Buy lots
        const Side closeSide = (incomingSide == Side::Buy) ? Side::Sell : Side::Buy;

        while (incomingQty.units > 0.0) {
            // Find first lot on the opposite side
            auto it = std::find_if(lots.begin(), lots.end(), [&](const PositionLot& L){
                return L.side == closeSide && L.qty.units > 0.0;
            });
            if (it == lots.end()) break; // nothing left to close

            const double matchQty = std::min(incomingQty.units, it->qty.units);

            // Realized PNL formula
            // - Closing LONG lot with a SELL at Px_s: (Px_s - entryPx) * Q
            // - Closing SHORT lot with a BUY  at Px_b: (entryPx - Px_b) * Q
            double pnl = 0.0;
            if (it->side == Side::Buy) {
                pnl = (incomingPx.px - it->entryPx.px) * matchQty;
            } else {
                pnl = (it->entryPx.px - incomingPx.px) * matchQty;
            }
            realized.amount += pnl;

            // Reduce the matched lot and incoming
            it->qty.units -= matchQty;
            incomingQty.units -= matchQty;

            // Clean up any fully closed lots at the front.
            while (!lots.empty() && lots.front().qty.units <= 0.0) {
                lots.pop_front();
            }
        }
    }

    void Position::onFill(Side side, Qty fillQty, Price fillPx, Timestamp ts) {
        // Try to close against opposite lots (realize PnL)
        closeAgainstFIFOLots(lots, side, fillQty, fillPx, realized);

        // Remaining quantity (if any) *opens* a new lot on the incoming side
        if (fillQty.units > 0.0) {
            PositionLot lot;
            lot.side    = side;
            lot.qty     = fillQty;
            lot.entryPx = fillPx;
            lot.ts      = ts;
            lots.push_back(lot);
        }

        // Recompute derived fields
        recomputeAverages_();
    }

    void Position::recomputeAverages_() {
        // Recompute netQty and avgPx from open lots only.
        double longQty = 0.0;
        double shortQty = 0.0;
        double longPV  = 0.0; // price * qty sums
        double shortPV  = 0.0;

        for (const auto& L : lots) {
            if (L.qty.units <= 1e-12) continue; // ignore fully-closed numerically
            if (L.side == Side::Buy) {
                longQty += L.qty.units;
                longPV  += L.entryPx.px * L.qty.units;
            } else {
                shortQty += L.qty.units;
                shortPV  += L.entryPx.px * L.qty.units;
            }
        }

        netQty = longQty - shortQty;

        if (netQty > 0.0 && longQty > 0.0) {
            avgPx.px = longPV / longQty;      // average entry for remaining longs
        } else if (netQty < 0.0 && shortQty > 0.0) {
            avgPx.px = shortPV / shortQty;    // average entry for remaining shorts
        } else {
            avgPx.px = 0.0; // flat
        }
    }

    Money Position::unrealized(Price mark) const {
        if (netQty == 0.0) return Money{currency, 0.0};

        // For longs: (mark - avg) * qty
        // For shorts: (avg - mark) * |qty|
        double q = std::abs(netQty);
        double pnl = (netQty > 0.0)? (mark.px - avgPx.px) * q : (avgPx.px - mark.px) * q;

        return Money{currency, pnl};
    }
}
