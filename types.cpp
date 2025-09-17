#include "types.h"
#include <stdexcept>
#include <sstream>
#include <iomanip>

namespace pms
{
    Money money(const std::string& ccy, double amount)
    {
        return Money{ccy, amount};
    }

    Price price(double p)
    {
        return Price{p};
    }

    Qty qty(double q)
    {
        return Qty{q};
    }

    Money notional(Price p, Qty q, const std::string& ccy)
    {
        return Money{ccy, p.px * q.units};
    }

    Money unrealized_pnl(Side side, Price entry, Price mark, Qty q, const std::string& ccy)
    {
        const double diff = mark.px - entry.px;
        double pnl = (side == Side::Buy) ? (diff * q.units) : (-diff * q.units);
        return Money{ccy, pnl};
    }

    // Fixed-point notation
    // Number of digits after the decimal is according to setprecision()
    std::string to_string(const Money& m)
    {
        std::ostringstream oss;
        oss << m.ccy << ' ' << std::fixed << std::setprecision(2) << m.amount;
        return oss.str();
    }

    std::string to_string(const Price& p)
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6) << p.px;
        return oss.str();
    }

    std::string to_string(const Qty& q)
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6) << q.units;
        return oss.str();
    }
}