#include "../headers/ItchTranslator.h";

ItchTranslator::ItchTranslator(OrderBook& book) : book_(book) {}

Side ItchTranslator::to_side(char indicator) {
    return indicator == 'B' ? Side::Buy : Side::Sell;
}

void ItchTranslator::process(const itch::Message& msg) {
    std::visit([this](auto&& m) {
        using T = std::decay_t<decltype(m)>;

        if constexpr (std::is_same_v<T, itch::AddOrderMessage>) {
            handle_add(m);
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

void ItchTranslator::handle_add(const itch::AddOrderMessage& m) {
    auto order = std::make_shared<Order>(
        OrderType::GoodTillCancel,
        m.order_reference_number,
        to_side(m.buy_sell_indicator),
        m.price,
        m.shares
    );
    book_.AddOrder(order);
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

void ItchTranslator::handle_execute(const itch::OrderExecutedMessage& m) {
    book_.CancelOrder(m.order_reference_number);
}

void ItchTranslator::handle_execute(const itch::OrderExecutedWithPriceMessage& m) {
    book_.CancelOrder(m.order_reference_number);
}

void ItchTranslator::handle_cancel(const itch::OrderCancelMessage& m) {
    book_.CancelOrder(m.order_reference_number);
}

void ItchTranslator::handle_delete(const itch::OrderDeleteMessage& m) {
    book_.CancelOrder(m.order_reference_number);
}

void ItchTranslator::handle_replace(const itch::OrderReplaceMessage& m) {
    OrderModify modify{
        m.original_order_reference_number,
        Side::Buy,  // book looks up actual side internally
        m.price,
        m.shares
    };
    book_.ModifyOrder(modify);
}