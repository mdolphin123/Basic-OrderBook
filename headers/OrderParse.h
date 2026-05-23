#pragma once
#include "OrderBook.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string_view>
#include <charconv>
#include <tuple>
#include <filesystem>
#include <format>
#include <stdexcept>


enum class ActionType { //represents what actions happen in the orderbook
    Add, 
    Modify,
    Cancel
};


struct Information { //Information needed for an order + action! ActionType type_ line is for the action and the rest are for creating the order
    ActionType type_;
    OrderType orderType_;
    Side side_;
    Price price_;
    Quantity quantity_;
    OrderId orderId_;
};

using Informations = std::vector<Information>; //stores incremental states, want to compare to results


//Stores expected result of a test, count all orders as well as bid and ask orders
struct Result {
    std::size_t allCount_;
    std::size_t bidCount_;
    std::size_t askCount_;
};


struct InputHandler { //Converts from text inputs into C++ orderbook actions/objects
    private:
        std::uint32_t ToNumber(const std::string_view& str) const;
        bool TryParseResult(const std::string_view& str, Result& result) const;
        bool TryParseInformation (const std::string_view& str, Information& info) const;

        std::vector<std::string_view> Split(const std::string_view& str, char delimeter) const;

        Side ParseSide(const std::string_view& str) const;
        OrderType ParseOrderType(const std::string_view& str) const;
        Price ParsePrice(const std::string_view& str) const;
        Quantity ParseQuantity(const std::string_view& str) const;
        OrderId ParseOrderId(const std::string_view& str) const;

    public:
        std::tuple<Informations, Result> GetInformations(const std::filesystem::path& path) const;
};