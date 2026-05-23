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


int main() {
    OrderBook orderbook;
    const OrderId orderId = 1;
    orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, orderId, Side::Buy, 100, 10));
    std::cout <<orderbook.Size() <<std::endl; //endl means end line, print a new line!
    orderbook.CancelOrder(orderId);
    std::cout <<orderbook.Size() <<std::endl;
    return 0;
}
    
        


