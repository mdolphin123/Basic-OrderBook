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

#include "../headers/Constants.h"
#include "../headers/OrderType.h"
#include "../headers/Usings.h"
#include "../headers/LevelInfo.h"
#include "../headers/Side.h"
#include "../headers/OrderBookLevelInfos.h"
#include "../headers/TradeInfo.h"
#include "../headers/Order.h"
#include "../headers/OrderModify.h"
#include "../headers/Trade.h"
#include "../headers/OrderBook.h"
#include "../headers/ItchTranslator.h"
#include "../headers/Itchparser.h"


int main() {
    // create the three objects needed
    itch::Parser parser;
    OrderBook book;
    ItchTranslator bridge(book);

    // open the ITCH data file
    std::ifstream file("/Users/jocelyn/Downloads/tvagg", std::ios::binary);
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

