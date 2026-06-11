#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

//define all message structs!
namespace itch {
    #pragma pack(push, 1)  //disable padding, NASDAQ doesnt use padding

    struct SystemEventMessage { //Struct fo system message!
        char message_type = 'S';
        uint16_t stock_locate; //which stock this message goes to
        uint16_t tracking_number; //Nasdaq internal sequence number
        uint64_t timestamp; //time since midnight msg was generated
        char event_code; //market event that happened
    }

    struct StockDirectoryMessage { //Stock directory message
        char message_type = 'R';
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        char stock[8];
        char market_category;
        char financial_status_indicator;
        uint32_t round_lot_size;
        char round_lots_only;
        char issue_classification;
        char issue_sub_type[2];
        char authenticity;
        char short_sale_threshold_indicator;
        char ipo_flag;
        char luld_ref;
        char etp_flag;
        uint32_t etp_leverage_factor;
        char inverse_indicator;
};

}