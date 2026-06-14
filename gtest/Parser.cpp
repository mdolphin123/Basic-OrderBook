#include "../headers/OrderBook.h"
#include "../headers/OrderParse.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string_view>
#include <charconv>
#include <tuple>
#include <filesystem>
#include <format>
#include <stdexcept>

#include "../headers/Itchparser.h"
#include "../headers/Indicators.h"
#include "../headers/Messages.h"




std::uint32_t InputHandler::ToNumber(const std::string_view& str) const { 
    std::int64_t value{}; //creates the variable value and allocates memory (64 bits)
    std::from_chars(str.data(), str.data() + str.size(), value); //read characters from these indices and store them into value (int)
    if(value < 0) {
        throw std::logic_error("Value is below zero");
    }
    return static_cast<std::uint64_t>(value); //convert to integer type
}

bool InputHandler::TryParseResult(const std::string_view& str, Result& result) const { //take test from text file and turn it into real numeric values, 
    //pass in an empty result struct, then fill it!
    if(str.at(0) != 'R') {
        return false;
    }
    auto values = Split(str, ' ');
    result.allCount_ = ToNumber(values.at(1));
    result.bidCount_ = ToNumber(values.at(2));
    result.askCount_ = ToNumber(values.at(3));

    return true; //return that the result struct was successfully filled, and the struct is also edited
}


bool InputHandler::TryParseInformation (const std::string_view& str, Information& info) const { //Converts from text to information struct
    auto value = str.at(0); //take the first character of input line, command we are doing
    auto values = Split(str, ' '); //now split the line
    if(value == 'A') { //add order!
        info.type_ = ActionType::Add;
        info.side_ = ParseSide(values.at(1));
        info.orderType_ = ParseOrderType(values.at(2));
        info.price_ = ParsePrice(values.at(3));
        info.quantity_ = ParseQuantity(values.at(4));
        info.orderId_ = ParseOrderId(values.at(5));
    }
    else if(value == 'M') { //need to be consistent with fields in the file, type is already known
        info.type_ = ActionType::Modify;
        info.orderId_ = ParseOrderId(values.at(1));
        info.side_ = ParseSide(values.at(2));
        info.price_ = ParsePrice(values.at(3));
        info.quantity_ = ParseQuantity(values.at(4));

    }
    else if(value == 'C') { //only care about the Id
        info.type_ = ActionType::Cancel;
        info.orderId_ = ParseOrderId(values.at(1));
    }
    else { //didn't successfully create the info struct
        return false;
    }
    return true; //successfully created it
}
        

std::vector<std::string_view> InputHandler::Split(const std::string_view& str, char delimeter) const { //C++ has no split function!
    std::vector<std::string_view> columns{}; //empty vector columns that stores the pieces
    std::size_t startIndex{}, endIndex{}; //creates variables start and end index that track string position, unsigned
    while((endIndex = str.find(delimeter, startIndex)) && endIndex != std::string::npos) { //keep looping while we still have delimeters
        auto distance = endIndex - startIndex;
        auto column = str.substr(startIndex, distance);
        startIndex = endIndex + 1;
        columns.push_back(column);
    }
        columns.push_back(str.substr(startIndex)); 
        return columns; //return the list
}


Side InputHandler::ParseSide(const std::string_view& str) const { //converts into side enum in orderbook class
    if(str == "B") {
        return Side::Buy;
    }
    else if(str == "S") {
        return Side::Sell;
    }
    else {
        throw std::logic_error("Unknown Side");
    }
}

OrderType InputHandler::ParseOrderType(const std::string_view& str) const { //for the OrderType enum
    if(str == "FillandKill") {
        return OrderType::FillandKill;
    }
    else if(str == "GoodTillCancel") {
        return OrderType::GoodTillCancel;
    }
    else if(str == "GoodForDay") {
        return OrderType::GoodForDay;
    }
    else if(str == "FillorKill") {
        return OrderType::FillorKill;
    }
    else if(str == "Market") {
        return OrderType::Market;
    }
    else throw std::logic_error("Unknown Order Type");
}


Price InputHandler::ParsePrice(const std::string_view& str) const { //parses price field from the test file, converts it to number
    if(str.empty()) {
        throw std::logic_error("Unknown Price");
    }
    return ToNumber(str);
}


Quantity InputHandler::ParseQuantity(const std::string_view& str) const { //parses quantity from test file, converts it to number
    if(str.empty()) {
        throw std::logic_error("Unknown Quantity");
    }
    return ToNumber(str);
}

OrderId InputHandler::ParseOrderId(const std::string_view& str) const { //parses orderId
    if(str.empty()) {
        throw std::logic_error("Unknown Quantity");
    }
    return ToNumber(str);

}

std::tuple<Informations, Result> InputHandler::GetInformations(const std::filesystem::path& path) const {
    //main parser function! Input path to test file
    Informations infos; //stores all parsed orders from the file
    infos.reserve(1'000); //preallocate memory

    std::string line; //temporary variable to store one line in the file
    std::ifstream file { path }; //open file for reading
            
    while(std::getline(file, line)) { //reads the file line by line
        if(line.empty()) {
            break;
        }

        //check if it is a result line
        const bool isResult = line.at(0) == 'R';
        const bool isUpdate = !isResult;

        if (isUpdate) { //for updates
            Information update; //create information struct

            auto isValid = TryParseInformation(line, update); //creates the struct and returns if successful
            if(!isValid) { //not successful
                throw std::logic_error(std::format("Invalid update: {}", line));
            }
            infos.push_back(update); //otherwise add it
        }
        else { //for the results line
            if(!file.eof()) {
                throw std::logic_error("Result must be at the end of the file only.");
            }
            Result result;

            auto isValid = TryParseResult(line, result); //create result struct
            if(!isValid) {
                continue;
            }
            return {infos, result}; //return both infos and result together as a tuple
        }
    }
    throw std::logic_error("No result specified.");

}
 
