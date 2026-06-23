#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <variant>
#include <vector>


//define all message structs! Basically dictionary of all ITCH message types, transalted -> C++ struct
namespace itch {
    #pragma pack(push, 1)  //disable padding, NASDAQ doesnt use padding

    struct SystemEventMessage { //Struct fo system message!
        char message_type = 'S';
        uint16_t stock_locate; //which stock this message goes to
        uint16_t tracking_number; //Nasdaq internal sequence number
        uint64_t timestamp; //time since midnight msg was generated
        char event_code; //market event that happened
    };

    struct StockDirectoryMessage { //Stock directory message in memory
        char message_type = 'R'; //Stock directory in ITCH
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

    struct StockTradingActionMessage {
        char message_type = 'H';
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        char stock[8];
        char trading_state;
        char reserved;
        char reason[4];
    };

    struct RegSHOMessage {
        char message_type = 'Y';
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        char stock[8];
        char reg_sho_action;
    };

    struct MarketParticipantPositionMessage {
        char message_type = 'L';
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        char mpid[4];
        char stock[8];
        char primary_market_maker;
        char market_maker_mode;
        char market_participant_state;
    };
    
    struct MWCBDeclineLevelMessage {
        char message_type = 'V';
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        uint64_t level1;
        uint64_t level2;
        uint64_t level3;
    };

    struct MWCBStatusMessage {
        char message_type = 'W';
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        char breached_level;
    };

    struct IPOQuotingPeriodUpdateMessage {
        char message_type = 'K';
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        char stock[8];
        uint32_t ipo_quotation_release_time;
        char ipo_quotation_release_qualifier;
        uint32_t ipo_price;
    };

    struct LULDAuctionCollarMessage {
        char message_type = 'J';
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        char stock[8];
        uint32_t auction_collar_reference_price;
        uint32_t upper_auction_collar_price;
        uint32_t lower_auction_collar_price;
        uint32_t auction_collar_extension;
    };

    struct OperationalHaltMessage {
        char message_type = 'h';
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        char stock[8];
        char market_code;
        char operational_halt_action;
    };

    struct AddOrderMessage {
        char message_type = 'A';
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        uint64_t order_reference_number;
        char buy_sell_indicator;
        uint32_t shares;
        char stock[8];
        uint32_t price;
    };

    struct AddOrderMPIDAttributionMessage {
        char message_type = 'F';
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        uint64_t order_reference_number;
        char buy_sell_indicator;
        uint32_t shares;
        char stock[8];
        uint32_t price;
        char attribution[4];
    };

    struct OrderExecutedMessage {
        char message_type = 'E';
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        uint64_t order_reference_number;
        uint32_t executed_shares;
        uint64_t match_number;
    };

    struct OrderExecutedWithPriceMessage {
        char message_type = 'C';
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        uint64_t order_reference_number;
        uint32_t executed_shares;
        uint64_t match_number;
        char printable;
        uint32_t execution_price;
    };

    struct OrderCancelMessage {
        char message_type = 'X';
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        uint64_t order_reference_number;
        uint32_t cancelled_shares;
    };

    struct OrderDeleteMessage {
        char message_type = 'D';
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        uint64_t order_reference_number;
    };

    struct OrderReplaceMessage {
        char message_type = 'U';
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        uint64_t original_order_reference_number;
        uint64_t new_order_reference_number;
        uint32_t shares;
        uint32_t price;
    };

    struct NonCrossTradeMessage {
        char message_type = 'P';
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        uint64_t order_reference_number;
        char buy_sell_indicator;
        uint32_t shares;
        char stock[8];
        uint32_t price;
        uint64_t match_number;
    };

    struct CrossTradeMessage {
        char message_type = 'Q';
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        uint64_t shares;
        char stock[8];
        uint32_t cross_price;
        uint64_t match_number;
        char cross_type;
    };

    struct BrokenTradeMessage {
        char message_type = 'B';
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        uint64_t match_number;
    };

    struct NOIIMessage {
        char message_type = 'I';
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        uint64_t paired_shares;
        uint64_t imbalance_shares;
        char imbalance_direction;
        char stock[8];
        uint32_t far_price;
        uint32_t near_price;
        uint32_t current_reference_price;
        char cross_type;
        char price_variation_indicator;
    };

    struct RetailPriceImprovementIndicatorMessage {
        char message_type = 'N';
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        char stock[8];
        char interest_flag;
    };

    struct DLCRMessage {
        char message_type = 'O';
        uint16_t stock_locate;
        uint16_t tracking_number;
        uint64_t timestamp;
        char stock[8];
        char open_eligibility_status;
        uint32_t minimum_allowable_price;
        uint32_t maximum_allowable_price;
        uint32_t near_execution_price;
        uint64_t near_execution_time;
        uint32_t lower_price_range_collar;
        uint32_t upper_price_range_collar;
    };

    #pragma pack(pop) //restores saved padding setting -> back to default/normal padding!

    //all types the message variable can hold
    using Message = std::variant<
        SystemEventMessage,
        StockDirectoryMessage,
        StockTradingActionMessage,
        RegSHOMessage,
        MarketParticipantPositionMessage,
        MWCBDeclineLevelMessage,
        MWCBStatusMessage,
        IPOQuotingPeriodUpdateMessage,
        LULDAuctionCollarMessage,
        OperationalHaltMessage,
        AddOrderMessage,
        AddOrderMPIDAttributionMessage,
        OrderExecutedMessage,
        OrderExecutedWithPriceMessage,
        OrderCancelMessage,
        OrderDeleteMessage,
        OrderReplaceMessage,
        NonCrossTradeMessage,
        CrossTradeMessage,
        BrokenTradeMessage,
        NOIIMessage,
        RetailPriceImprovementIndicatorMessage,
        DLCRMessage>;

        //compile time constants! Used wherever prices or stock tickers need to be handled
        constexpr int STOCK_LEN = 8;
        constexpr double PRICE_DIVISOR = 10000.0;
        constexpr double MWCB_PRICE_DIVISOR = 1.0E8;


        //trim trailing spaces from tickers, get rid of spaces for better output!
        inline auto to_string(const char* arr, size_t size) -> std::string {
            size_t len = size;
            while (len > 0 && (arr[len - 1] == ' ' || arr[len - 1] == '\0')) {
                len--;
            }
            return std::string {arr, len};
        }

        //forward declarations, print_impl is for printing (one per message type that knows how to print that specific function!)
        //start with where to print, then what to print, impl means implementation
        auto print_impl(std::ostream& out, const SystemEventMessage& msg) -> void;
        auto print_impl(std::ostream& out, const StockDirectoryMessage& msg) -> void;
        auto print_impl(std::ostream& out, const StockTradingActionMessage& msg) -> void;
        auto print_impl(std::ostream& out, const RegSHOMessage& msg) -> void;
        auto print_impl(std::ostream& out, const MarketParticipantPositionMessage& msg) -> void;
        auto print_impl(std::ostream& out, const MWCBDeclineLevelMessage& msg) -> void;
        auto print_impl(std::ostream& out, const MWCBStatusMessage& msg) -> void;
        auto print_impl(std::ostream& out, const IPOQuotingPeriodUpdateMessage& msg) -> void;
        auto print_impl(std::ostream& out, const LULDAuctionCollarMessage& msg) -> void;
        auto print_impl(std::ostream& out, const OperationalHaltMessage& msg) -> void;
        auto print_impl(std::ostream& out, const AddOrderMessage& msg) -> void;
        auto print_impl(std::ostream& out, const AddOrderMPIDAttributionMessage& msg) -> void;
        auto print_impl(std::ostream& out, const OrderExecutedMessage& msg) -> void;
        auto print_impl(std::ostream& out, const OrderExecutedWithPriceMessage& msg) -> void;
        auto print_impl(std::ostream& out, const OrderCancelMessage& msg) -> void;
        auto print_impl(std::ostream& out, const OrderDeleteMessage& msg) -> void;
        auto print_impl(std::ostream& out, const OrderReplaceMessage& msg) -> void;
        auto print_impl(std::ostream& out, const NonCrossTradeMessage& msg) -> void;
        auto print_impl(std::ostream& out, const CrossTradeMessage& msg) -> void;
        auto print_impl(std::ostream& out, const BrokenTradeMessage& msg) -> void;
        auto print_impl(std::ostream& out, const NOIIMessage& msg) -> void;
        auto print_impl(std::ostream& out, const RetailPriceImprovementIndicatorMessage& msg) -> void;
        auto print_impl(std::ostream& out, const DLCRMessage& msg) -> void;
        //good for logging + debugging!


        //Takes message variant and figures out which print functino to call
        auto print_message(std::ostream& out, const Message& msg) -> void;

        //prints an itch message
        auto operator<<(std::ostream& out, const Message& msg) -> std::ostream&;

}


