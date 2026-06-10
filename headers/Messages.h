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

}