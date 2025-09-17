#ifndef PMS_TYPES_H
#define PMS_TYPES_H

#include <string>
#include <chrono>

namespace pms
{
    // Aliases for convenience
    using InstrumentId = std::string;
    using OrderId = std::string;
    using TradeId = std::string;
    using AccountId = std::string;
    using Timestamp = std::chrono::system_clock::time_point; // Not implemented yet

    enum class Side
    {
        Buy,
        Sell
    };

    // Primitive types
    struct Money
    {
        std::string ccy; // currency
        double amount{};
    };

    struct Price
    {
        double px{};
    };

    struct Qty
    {
        double units{};
    };

    // Convenience constructors
    Money money(const std::string& ccy, double amount);
    Price price(double p);
    Qty qty(double q);

    // Finance helpers
    // Notional cash flow of a trade at a given price and quantity
    Money notional(Price p, Qty q, const std::string& ccy);

    // Unrealized PnL for a position (positive for gains, negative for losses)
    Money unrealized_pnl(Side side, Price entry, Price mark, Qty q, const std::string& ccy);

    // Format helpers
    std::string to_string(const Money& m);
    std::string to_string(const Price& p);
    std::string to_string(const Qty& q);

}

#endif