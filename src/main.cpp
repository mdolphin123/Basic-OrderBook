#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <stdexcept>
#include <numeric>
#include <memory>
#include <variant>
#include <optional>
#include <tuple>
#include <format>
#include <list>
#include <vector>
#include <map>
#include <cstdint>
#include <cmath>
#include <fstream>

#include "Constants.h"
#include "OrderType.h"
#include "Usings.h"
#include "LevelInfo.h"
#include "Side.h"
#include "OrderBookLevelInfos.h"
#include "TradeInfo.h"
#include "Order.h"
#include "OrderModify.h"
#include "Trade.h"
#include "OrderBook.h"
#include "ItchTranslator.h"
#include "Itchparser.h"


int main() {
    // create the three objects needed
    itch::Parser parser;
    OrderBook book;
    ItchTranslator bridge(book);

    // open the ITCH data file
    std::ifstream file("nasdaq_itch.bin", std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open ITCH file\n";
        return 1;
    }

    // parse every message and feed into order book
    parser.parse(file, itch::MessageCallback{[&](const itch::Message& msg) {
        bridge.process(msg);
    }});

    // print latency results after all messages processed
    book.PrintLatencyStats();

    return 0;
}

