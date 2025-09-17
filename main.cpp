#include "types.h"       // Money/Price/Qty + helpers
#include "instrument.h"  // Instrument + InstrumentRepo
#include "marketdata.h"  // Quote + MarketDataFeed + mid()
#include "position.h"    // Position (FIFO) + PnL

#include <iostream>
#include <sstream>
#include <vector>
#include <thread>
#include <chrono>
#include <cassert>

using namespace pms;

// Helper functions for output formatting
static void print_quote_line(const Quote& q, Price entry, Qty posQty) {
    const Price m = mid(q);
    const Money mv   = notional(m, posQty, "USD");
    const Money upnl = unrealized_pnl(Side::Buy, entry, m, posQty, "USD");

    std::cout << "[QUOTE " << q.id << "] "
              << "bid=" << to_string(q.bid)
              << " ask=" << to_string(q.ask)
              << " last=" << to_string(q.last)
              << " mid=" << to_string(m)
              << " | notional @ " << to_string(m) << "=" << to_string(mv)
              << " | uPnL(long " << to_string(posQty) << " @ " << to_string(entry) << ")="
              << to_string(upnl) << "\n";
}

static void show_position(const Position& pos, const Quote* lastQ) {
    const Price mark = lastQ ? mid(*lastQ) : pos.avgPx;
    std::cout << "---- POSITION " << pos.instrument << " ----\n"
              << "Currency : " << pos.currency << "\n"
              << "Net Qty  : " << pos.netQty << "\n"
              << "Avg Px   : " << to_string(pos.avgPx) << "\n"
              << "Realized : " << to_string(pos.realized) << "\n"
              << "Unrealized @" << to_string(mark) << " : " << to_string(pos.unrealized(mark)) << "\n\n";
}

int main() {

    // Load instruments from file else fallback to CSV in-memory
    InstrumentRepo repo;
    std::cout << "Instruments loaded from: ";
    try {
        std::string path = "data/instruments.csv";
        repo.loadFromCSVFile(path);
        std::cout << path << "\n";
    } catch (...) {
        const std::string csv =
            "id,symbol,assetClass,currency,tickSize\n"
            "AAPL,AAPL,Equity,USD,0.01\n"
            "MSFT,MSFT,Equity,USD,0.01\n"
            "ESZ5,ESZ5,Futures,USD,0.25\n";
        std::istringstream ss(csv);
        repo.loadFromCSVStream(ss);
        std::cout << "in-memory CSV\n";
    }
    std::cout << "Number of instruments loaded: " << repo.size() << "\n";

    // Load AAPL metadata from the instrument repository
    if (auto aapl = repo.get("AAPL")) {
        std::cout << "AAPL: " << aapl->assetClass << " " << aapl->currency << " (tick=" << aapl->tickSize << ")\n\n";
    }


    // Market data feed to receive quotes
    MarketDataFeed feed;
    const InstrumentId ID = "AAPL";

    const Price entry = price(100.00); // long entry
    const Qty   q10   = qty(10);       // position size for unrealized PnL

    // Subscribe to AAPL quotes and print metrics
    feed.subscribe(ID, [&](const Quote& q){ print_quote_line(q, entry, q10); });

    // Push a few quotes (normally read from CSV or a socket)
    std::vector<Quote> ticks = {
        Quote{ ID, price(189.70), qty(200), price(189.76), qty(150), price(189.73), std::chrono::system_clock::now() },
        Quote{ ID, price(189.72), qty(220), price(189.78), qty(120), price(189.75), std::chrono::system_clock::now() + std::chrono::milliseconds(200) },
        Quote{ ID, price(189.90), qty(180), price(189.96), qty(160), price(189.94), std::chrono::system_clock::now() + std::chrono::milliseconds(400) },
    };
    for (auto& q : ticks) {
        feed.push(q);
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
    }

    if (auto* last = feed.last(ID)) {
        std::cout << "Last cached mid: " << to_string(mid(*last)) << "\n\n";
    }


    // Position for AAPL
    Position pos;
    pos.instrument = ID;
    pos.currency   = "USD";

    // FIFO clearing of lots
    // Buy 10 @ 100; Buy 10 @ 110; Sell 15 @ 120
    pos.onFill(Side::Buy,  qty(10), price(100.00));
    pos.onFill(Side::Buy,  qty(10), price(110.00));
    pos.onFill(Side::Sell, qty(15), price(120.00));

    // Realized should be (120-100)*10 + (120-110)*5 = 250
    assert(std::abs(pos.realized.amount - 250.0) < 1e-9 && "Realized PnL should be 250.00");

    show_position(pos, feed.last(ID)); // unrealized uses last mid if available

    // Short 5 @ 105
    pos.onFill(Side::Sell, qty(5), price(105.00));
    show_position(pos, feed.last(ID));

    // Short 8 @ 119
    pos.onFill(Side::Sell, qty(8), price(119.00));
    show_position(pos, feed.last(ID));

    // Buy 3 @ 112
    pos.onFill(Side::Buy, qty(3), price(112.00));
    show_position(pos, feed.last(ID));

    std::cout << "Demo complete.\n";
    return 0;
}
