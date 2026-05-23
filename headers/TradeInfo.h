#pragma once
#include "Usings.h"

//Now we are handling actual exchanges! Before was order book structure
struct TradeInfo { //trade info structure
    OrderId orderId_;
    Price price_;
    Quantity quantity;
};