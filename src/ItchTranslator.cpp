#include "../headers/ItchTranslator.h";
#include <variant>
#include <type_traits> 
#include <memory> 

ItchTranslator::ItchTranslator(OrderBook& book) : book_(book) {}

Side ItchTranslator::to_side(char indicator) {
    if (indicator == 'B') {
        return Side::Buy;
    }
    else {
        return Side::Sell;
    }
}

void ItchTranslator::process(const itch::Message& msg) { //takes itch message
    std::visit([this](auto&& m) { //peeks inside message variant to see what is inside! Check for types that affect
        //the order book
        using T = std::decay_t<decltype(m)>; //get type name of m

        //check all the possible types
        if constexpr (std::is_same_v<T, itch::AddOrderMessage>) {
            handle_add(m); //called to deal with when an order gets traded
        }
        else if constexpr (std::is_same_v<T, itch::AddOrderMPIDAttributionMessage>) {
            handle_add(m);
        }
        else if constexpr (std::is_same_v<T, itch::OrderExecutedMessage>) {
            handle_execute(m);
        }
        else if constexpr (std::is_same_v<T, itch::OrderExecutedWithPriceMessage>) {
            handle_execute(m);
        }
        else if constexpr (std::is_same_v<T, itch::OrderCancelMessage>) {
            handle_cancel(m);
        }
        else if constexpr (std::is_same_v<T, itch::OrderDeleteMessage>) {
            handle_delete(m);
        }
        else if constexpr (std::is_same_v<T, itch::OrderReplaceMessage>) {
            handle_replace(m);
        }
        // all other message types ignored
    }, msg);
}

//handle adds to the book
void ItchTranslator::handle_add(const itch::AddOrderMessage& m) { //translate itch message into add order format
    auto order = std::make_shared<Order>(
        OrderType::GoodTillCancel,
        m.order_reference_number,
        to_side(m.buy_sell_indicator),
        m.price,
        m.shares
    );
    book_.AddOrder(order); //call add order format!
}

void ItchTranslator::handle_add(const itch::AddOrderMPIDAttributionMessage& m) {
    auto order = std::make_shared<Order>(
        OrderType::GoodTillCancel,
        m.order_reference_number,
        to_side(m.buy_sell_indicator),
        m.price,
        m.shares
    );
    book_.AddOrder(order);
}

//handle deletes from the book
void ItchTranslator::handle_execute(const itch::OrderExecutedMessage& m) { //order traded, remove from book
    book_.CancelOrder(m.order_reference_number);
}

void ItchTranslator::handle_execute(const itch::OrderExecutedWithPriceMessage& m) { //same thing as above!
    book_.CancelOrder(m.order_reference_number);
}

void ItchTranslator::handle_cancel(const itch::OrderCancelMessage& m) { //trader submitted cancel request, remove from book
    book_.CancelOrder(m.order_reference_number);
}

void ItchTranslator::handle_delete(const itch::OrderDeleteMessage& m) { //exchange removed the order!
    book_.CancelOrder(m.order_reference_number);
}

void ItchTranslator::handle_replace(const itch::OrderReplaceMessage& m) { //order modify message
    OrderModify modify{
        m.original_order_reference_number,
        Side::Buy,  // book looks up actual side internally
        m.price,
        m.shares
    };
    book_.ModifyOrder(modify); //modify the order
}