#pragma once
#include "OrderBook.h"
#include "Itchparser.h"


#pragma once
#include "OrderBook.h"
#include "Itchparser.h"

class ItchTranslator {
public:
    ItchTranslator(OrderBook& book); //take a reference to orderbook
    void process(const itch::Message& msg); //function that takes ITCH message and handles it

private:
    OrderBook& book_; //store the reference to orderbook
    Side to_side(char indicator); //converts ITCH to side

    void handle_add(const itch::AddOrderMessage& m); //handlers for different add order messages, eventually do addorder
    void handle_add(const itch::AddOrderMPIDAttributionMessage& m);
    void handle_execute(const itch::OrderExecutedMessage& m);
    void handle_execute(const itch::OrderExecutedWithPriceMessage& m);
    void handle_cancel(const itch::OrderCancelMessage& m);
    void handle_delete(const itch::OrderDeleteMessage& m);
    void handle_replace(const itch::OrderReplaceMessage& m);
};