#include <cstdint>
#include <cstring>
#include <set>
#include <stdexcept>
#include <vector>
#include <iostream>


#include "../headers/Itchparser.h"
#include "../headers/Indicators.h"
#include "../headers/Messages.h"

//use namespace so you can reuse variables, new C++ style
namespace itch {

namespace utils {
    //saves stack overhead, use inline
    //do the endian conversion in preprocessing to save time at runtime, check byte order once!
    inline auto is_little_endian() -> bool {
        #if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            return true;
        #elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            return false;
        #else
            const union {
                uint32_t i;
                char c[4];
            }
            bint = {0x01020304}; //fill the struct, hex notation!
            return bint.c[0] == 4; //if 4, then it is little endian
        #endif
    }

    template <typename T> //function takes any type and returns the same type, depends on machine!
    auto from_big_endian(T value) -> T { //convert to little endian if needed
        if(is_little_endian()) {
            return swap_bytes(value);
        }
        else {
            return value;
        }
    }

    //read one field out of raw itch buffer and convert it to the correct byte order
    //take in pointer to buffer, offset tracks current position in buffer
    template <typename T> 
    auto unpack(const char* buffer, size_t& offset) -> T {
        T value; //variable to hold the bytes
        std::memcpy(&value, buffer + offset, sizeof(T)); //copy size of T byte sinto variable value starting at buffer + offset
        //note buffer always points to the start
        offset += sizeof(T); //move offset

        if constexpr(std::is_integral_v<T> && sizeof(T) > 1) { //if it is a multibyte number, just convert it
            return from_big_endian(value);
        }
        else {
            return value;
        }
    }
    
    //for when there are text fields (e.g., stock tickets, commands)
    inline auto unpack_string(const char* buffer, size_t& offset, char* dest, size_t size) -> void {
        //dest is where to copy into!
        std::memcpy(dest, buffer + offset, size);
        offset += size; //update offset

    }

    inline auto unpack_timestamp(const char* buffer, size_t& offset) -> uint64_t {
        uint16_t high{}; //2 byte box
        uint16_t low{}; //4 byte box
        constexpr int lower_shift = 32; //named constant, we need to shift high left by this amouhnt
        std::memcpy(&high, buffer + offset, sizeof(high)); //copying into high
        offset += sizeof(high);
        std::memcpy(&low, buffer + offset, sizeof(low));
        offset += sizeof(low);
        

        //convert from big endian
        high = from_big_endian(high);
        low = from_big_endian(low); 

        return (static_cast<uint64_t>(high) << lower_shift) | low;


    }
} //end of namespace

auto unpack_message(SystemEventMessage& msg, const char* buffer, size_t& offset) -> void { //reads event code byte
    msg.event_code = utils::unpack<char>(buffer, offset);
}

auto unpack_message(StockDirectoryMessage& msg, const char* buffer, size_t& offset) -> void { //unpacks the stock
    //directory message, tells you state of market 
    utils::unpack_string(buffer, offset, msg.stock, STOCK_LEN);
    msg.market_category = utils::unpack<char>(buffer, offset);
    msg.financial_status_indicator = utils::unpack<char>(buffer, offset);
    msg.round_lot_size = utils::unpack<uint32_t> (buffer, offset);
    msg.round_lots_only = utils::unpack<char> (buffer, offset);
    msg.issue_classification = utils::unpack<char>(buffer, offset);

    utils::unpack_string(buffer, offset, msg.issue_sub_type, 2);
    msg.authenticity = utils::unpack<char>(buffer, offset);
    msg.short_sale_threshold_indicator = utils::unpack<char>(buffer, offset);
    msg.ipo_flag = utils::unpack<char> (buffer, offset);
    msg.luld_ref = utils::unpack<char> (buffer, offset);
    msg.etp_flag = utils::unpack<char> (buffer, offset);
    msg.etp_leverage_factor = utils::unpack<uint32_t> (buffer, offset);
    msg.inverse_indicator = utils::unpack<char> (buffer, offset);

}

//unpacks stock action trading message! Tells system when a stock's trading status changes during the day, e.g. stock gets halted
auto unpack_message(StockTradingActionMessage& msg, const char* buffer, size_t& offset) -> void {
    utils::unpack_string(buffer, offset, msg.stock, STOCK_LEN);
    msg.trading_state = utils::unpack<char>(buffer, offset);
    msg.reserved = utils::unpack<char>(buffer, offset);
    utils::unpack_string(buffer, offset, msg.reason, 4);

}

//unpacks Reg SHO message: tells you if your system if short selling is restricted for a certain stock!
auto unpack_message(RegSHOMessage& msg, const char* buffer, size_t& offset) -> void {
    utils::unpack_string(buffer, offset, msg.stock, STOCK_LEN);
    msg.reg_sho_action = utils::unpack<char>(buffer, offset);
}

//unpacks market position message: tells your system what role a specific market maker is playing for a stock
auto unpack_message(MarketParticipantPositionMessage& msg, const char* buffer, size_t& offset) -> void {
    utils::unpack_string(buffer, offset, msg.mpid, 4);
    utils::unpack_string(buffer, offset, msg.stock, STOCK_LEN);
    msg.primary_market_maker = utils::unpack<char>(buffer, offset);
    msg.market_maker_mode = utils::unpack<char>(buffer, offset);
    msg.market_participant_state = utils::unpack<char>(buffer, offset);
}

//This unpacks a Market Wide Circuit Breaker Decline Level Message, tells system at what price levels the entire market will be halted
auto unpack_message(MWCBDeclineLevelMessage& msg, const char* buffer, size_t& offset) -> void {
    msg.level1 = utils::unpack<uint64_t>(buffer, offset);
    msg.level2 = utils::unpack<uint64_t>(buffer, offset);
    msg.level3 = utils::unpack<uint64_t>(buffer, offset);
}

//This unpacks a Market Wide Circuit Breaker Status Level Message, tells system at what price levels the entire market will be halted
//Very similar to previous one!
auto unpack_message(MWCBStatusMessage& msg, const char* buffer, size_t& offset) -> void {
    msg.breached_level = utils::unpack<char>(buffer, offset);
}

//Tells system about stock IPO opening process
auto unpack_message(IPOQuotingPeriodUpdateMessage& msg, const char* buffer, size_t& offset) -> void {
    utils::unpack_string(buffer, offset, msg.stock, STOCK_LEN);
    msg.ipo_quotation_release_time = utils::unpack<uint32_t>(buffer, offset);
    msg.ipo_quotation_release_qualifier = utils::unpack<char>(buffer, offset);
    msg.ipo_price = utils::unpack<uint32_t>(buffer, offset);
}

//Defines price boundaries for a stock during Limit Up Limit Down halt auction
auto unpack_message(LULDAuctionCollarMessage& msg, const char* buffer, size_t& offset) -> void {
    utils::unpack_string(buffer, offset, msg.stock, STOCK_LEN);
    msg.auction_collar_reference_price = utils::unpack<uint32_t>(buffer, offset);
    msg.upper_auction_collar_price = utils::unpack<uint32_t>(buffer, offset);
    msg.lower_auction_collar_price = utils::unpack<uint32_t>(buffer, offset);
    msg.auction_collar_extension = utils::unpack<uint32_t>(buffer, offset);
}

//Unpacks operational halt message, tells system when stock is halted bc of technical or operational issue 
auto unpack_message(OperationalHaltMessage& msg, const char* buffer, size_t& offset) -> void {
    utils::unpack_string(buffer, offset, msg.stock, STOCK_LEN);
    msg.market_code = utils::unpack<char>(buffer, offset);
    msg.operational_halt_action = utils::unpack<char>(buffer, offset);
}
//Important!! AddOrder message, basically someone submits an order!
auto unpack_message(AddOrderMessage& msg, const char* buffer, size_t& offset) -> void {
    msg.order_reference_number = utils::unpack<uint64_t>(buffer, offset);
    msg.buy_sell_indicator = utils::unpack<char>(buffer, offset);
    msg.shares = utils::unpack<uint32_t>(buffer, offset);
    utils::unpack_string(buffer, offset, msg.stock, STOCK_LEN);
    msg.price = utils::unpack<uint32_t>(buffer, offset);
}

//Similar to previous (AddOrderMessage), but has extra attribution field at the end! Identifies which firm placed the order
auto unpack_message(AddOrderMPIDAttributionMessage& msg, const char* buffer, size_t& offset) -> void {
    msg.order_reference_number = utils::unpack<uint64_t>(buffer, offset);
    msg.buy_sell_indicator = utils::unpack<char>(buffer, offset);
    msg.shares = utils::unpack<uint32_t>(buffer, offset);

    utils::unpack_string(buffer, offset, msg.stock, STOCK_LEN);
    msg.price = utils::unpack<uint32_t>(buffer, offset);
    utils::unpack_string(buffer, offset, msg.attribution, 4);
}

//Also important! Tells your system that an order was actually traded
auto unpack_message(OrderExecutedMessage& msg, const char* buffer, size_t& offset) -> void {
    msg.order_reference_number = utils::unpack<uint64_t>(buffer, offset);
    msg.executed_shares = utils::unpack<uint32_t>(buffer, offset);
    msg.match_number = utils::unpack<uint64_t>(buffer, offset);
}

//Similar to order unpack, just with extra fields! Occurs if execution price differs from original orders
auto unpack_message(OrderExecutedWithPriceMessage& msg, const char* buffer, size_t& offset) -> void {
    msg.order_reference_number = utils::unpack<uint64_t>(buffer, offset);
    msg.executed_shares = utils::unpack<uint32_t>(buffer, offset);
    msg.match_number = utils::unpack<uint64_t>(buffer, offset);
    msg.printable = utils::unpack<char>(buffer, offset);
    msg.execution_price = utils::unpack<uint32_t>(buffer, offset);
}

//Order cancellation message! Cancels shares
auto unpack_message(OrderCancelMessage& msg, const char* buffer, size_t& offset) -> void {
    msg.order_reference_number = utils::unpack<uint64_t>(buffer, offset);
    msg.cancelled_shares = utils::unpack<uint32_t>(buffer, offset);
}

//Also an order cancellation message. Simply removes an order from the book 
auto unpack_message(OrderDeleteMessage& msg, const char* buffer, size_t& offset) -> void {
    msg.order_reference_number = utils::unpack<uint64_t>(buffer, offset);
}

//Order replacement aka order modification message! If trader modifies their message
auto unpack_message(OrderReplaceMessage& msg, const char* buffer, size_t& offset) -> void {
    msg.original_order_reference_number = utils::unpack<uint64_t>(buffer, offset);
    msg.new_order_reference_number = utils::unpack<uint64_t>(buffer, offset);
    msg.shares = utils::unpack<uint32_t>(buffer, offset);
    msg.price = utils::unpack<uint32_t>(buffer, offset);
}

//Unpacks non cross trade message, a trade that happened outside the regular orderbook
auto unpack_message(NonCrossTradeMessage& msg, const char* buffer, size_t& offset) -> void {
    msg.order_reference_number = utils::unpack<uint64_t>(buffer, offset);
    msg.buy_sell_indicator = utils::unpack<char>(buffer, offset);
    msg.shares = utils::unpack<uint32_t>(buffer, offset);
    utils::unpack_string(buffer, offset, msg.stock, STOCK_LEN);
    msg.price = utils::unpack<uint32_t>(buffer, offset);
    msg.match_number = utils::unpack<uint64_t>(buffer, offset);
}

//Unpacks cross trade message: a trade that happens during special auction period
auto unpack_message(CrossTradeMessage& msg, const char* buffer, size_t& offset) -> void {
    msg.shares = utils::unpack<uint64_t>(buffer, offset);
    utils::unpack_string(buffer, offset, msg.stock, STOCK_LEN);

    msg.cross_price = utils::unpack<uint32_t>(buffer, offset);
    msg.match_number = utils::unpack<uint64_t>(buffer, offset);
    msg.cross_type = utils::unpack<char>(buffer, offset);
}

//Cancels previously reported trade 
auto unpack_message(BrokenTradeMessage& msg, const char* buffer, size_t& offset) -> void {
    msg.match_number = utils::unpack<uint64_t>(buffer, offset);
}

//Unpacks net order imbalance indicator message! Tells system about order imbalances during auction periods
auto unpack_message(NOIIMessage& msg, const char* buffer, size_t& offset) -> void {
    msg.paired_shares = utils::unpack<uint64_t>(buffer, offset);
    msg.imbalance_shares = utils::unpack<uint64_t>(buffer, offset);
    msg.imbalance_direction = utils::unpack<char>(buffer, offset);

    utils::unpack_string(buffer, offset, msg.stock, STOCK_LEN);

    msg.far_price = utils::unpack<uint32_t>(buffer, offset);
    msg.near_price = utils::unpack<uint32_t>(buffer, offset);
    msg.current_reference_price = utils::unpack<uint32_t>(buffer, offset);
    msg.cross_type = utils::unpack<char>(buffer, offset);
    msg.price_variation_indicator = utils::unpack<char>(buffer, offset);
}

//Unpacks Retail Price Improvement Indicator Message! Tells retail traders when they can
//get a better price than displayed in the book
using RPIMsg = RetailPriceImprovementIndicatorMessage;
auto unpack_message(RPIMsg& msg, const char* buffer, size_t& offset) -> void {
    utils::unpack_string(buffer, offset, msg.stock, STOCK_LEN);
    msg.interest_flag = utils::unpack<char>(buffer, offset);
}

//Unpacks direct listing capital raise message: for companies doing a direct listing on Nasdaq
auto unpack_message(DLCRMessage& msg, const char* buffer, size_t& offset) -> void {
    utils::unpack_string(buffer, offset, msg.stock, STOCK_LEN);
    msg.open_eligibility_status = utils::unpack<char>(buffer, offset);
    msg.minimum_allowable_price = utils::unpack<uint32_t>(buffer, offset);
    msg.maximum_allowable_price = utils::unpack<uint32_t>(buffer, offset);
    msg.near_execution_price = utils::unpack<uint32_t>(buffer, offset);
    msg.near_execution_time = utils::unpack<uint64_t>(buffer, offset);
    msg.lower_price_range_collar = utils::unpack<uint32_t>(buffer, offset);
    msg.upper_price_range_collar = utils::unpack<uint32_t>(buffer, offset);
}

//Register handler for each message type!
template <typename T> //T is the message struct, type is the char that identifies it
auto Parser::register_handler(char type) -> void {
    m_handlers[type] = [](const char* buffer) -> Message { //lambda function in handlers map, m_handlers is a map!
        //key of m_handlers is the message type, value is the function that knows how to unpack the msg
        T msg; //create empty struct of what type T is 
        size_t offset = 1; //skip byte 0, the message type we used already (read before function call)

        //common headers for every message!
        msg.stock_locate = utils::unpack_timestamp(buffer, offset);
        msg.tracking_number = utils::unpack_timestamp(buffer, offset);
        msg.timestamp = utils::unpack_timestamp(buffer, offset);


        unpack_message(msg, buffer, offset);
        return msg;
    };
}

//Parser constructor, register a handler for every single itch message type!
Parser::Parser() {
    register_handler<SystemEventMessage>('S');
    register_handler<StockDirectoryMessage>('R');
    register_handler<StockTradingActionMessage>('H');
    register_handler<RegSHOMessage>('Y');
    register_handler<MarketParticipantPositionMessage>('L');
    register_handler<MWCBDeclineLevelMessage>('V');
    register_handler<MWCBStatusMessage>('W');
    register_handler<IPOQuotingPeriodUpdateMessage>('K');
    register_handler<LULDAuctionCollarMessage>('J');
    register_handler<OperationalHaltMessage>('h');
    register_handler<AddOrderMessage>('A');
    register_handler<AddOrderMPIDAttributionMessage>('F');
    register_handler<OrderExecutedMessage>('E');
    register_handler<OrderExecutedWithPriceMessage>('C');
    register_handler<OrderCancelMessage>('X');
    register_handler<OrderDeleteMessage>('D');
    register_handler<OrderReplaceMessage>('U');
    register_handler<NonCrossTradeMessage>('P');
    register_handler<CrossTradeMessage>('Q');
    register_handler<BrokenTradeMessage>('B');
    register_handler<NOIIMessage>('I');
    register_handler<RetailPriceImprovementIndicatorMessage>('N');
    register_handler<DLCRMessage>('O');
    //register_handler creates a function + stores it in one line so parser can later look it up by message type
}


//Parse itch bytes into C++ ITCH structs (intermediate step before orderbook)
auto Parser::parse(const char* data, size_t size, const MessageCallback& callback) -> void {
    size_t offset = 0; //start of buffer
    while (offset < size) { //keep looping
        if (offset + sizeof(uint16_t) > size) { //make sure there are at least 2 bytes left!
            throw std::runtime_error("Incomplete message header at end of buffer.");
        }
        uint16_t length {}; //reads 2 length byte prefix of ITCH message
        std::memcpy(&length, data + offset, sizeof(length)); //copy 2 bytes into empty itch message

        length = utils::from_big_endian(length); //convert bytes from big to little endian
        offset += sizeof(uint16_t); //add to offset, move it forward

        if (length == 0) { //skip empty messages
            continue;
        }

        if (offset + length > size) { //make sure the full message fits
            throw std::runtime_error("Incomplete message at end of buffer.");
        }
        const char* message = data + offset; //points to start of current message
        const char  message_type = message[0]; //reads first byte of messages

        auto handler_it = m_handlers.find(message_type); //find the message type in the map
        if (handler_it != m_handlers.end()) {  //makes sure a handler was actually found
            callback(handler_it->second(message)); //passes filled struct to who called parse
        } else {
            std::cerr << "Unknown or unhandled message type: " << message_type << '\n'; //no handler found, log and keep going
        }
        offset += length; //move to the next message
    }
}
//overloaded, another parser that collects everything into a vector
constexpr size_t average_message_size = 20; //preallocate memory (guess)
auto Parser::parse(const char* data, size_t size) -> std::vector<Message> {
    std::vector<Message> messages; //vector of messages, this method just puts messages into a vector
    messages.reserve(size / average_message_size);
    parse(data, size, [&](const Message& msg) { messages.push_back(msg); });
    return messages; 
}

//another overloaded parser
auto Parser::parse(const char* data, size_t size, const std::vector<char>& messages) -> std::vector<Message> {
    std::vector<Message> results;
    std::set<char> filter(messages.begin(), messages.end()); //converts vector -> set

    if (filter.empty()) {
        return results;
    }
    results.reserve(size / average_message_size); 

    auto callback = [&](const Message& msg) {
        char message_type = std::visit([](auto&& arg) { return arg.message_type; }, msg); //filtering lambda
        if (filter.contains(message_type)) { //if msg type is contained, push it back
            results.push_back(msg);
        }
    };
    
    parse(data, size, callback); //main parse with filtering callback
    return results;
}

//reads entire file stream -> memory
static auto read_stream_into_buffer(std::istream& data) -> std::vector<char> {
    data.seekg(0, std::ios::end); //moves to end of stream to see how big it is
    auto size = data.tellg(); //gives file size
    data.seekg(0, std::ios::beg); //moves back to beginning
    if (size < 0) {
        throw std::runtime_error("Failed to determine stream size.");
    }
    std::vector<char> buffer(static_cast<size_t>(size)); //create vector to hold entire file
    data.read(buffer.data(), size); //reads entire file into buffer
    return buffer; //returns file as byte vector
}


//thin reader, connects stream reader -> parser
auto Parser::parse(std::istream& data, const MessageCallback& callback) -> void {
    auto buffer = read_stream_into_buffer(data); //loads entire file into memory
    parse(buffer.data(), buffer.size(), callback); //calls main parse with loaded bytes
}

//takes a stream, returns all msgs as vector
auto Parser::parse(std::istream& data) -> std::vector<Message> {
    auto buffer = read_stream_into_buffer(data);
    return parse(buffer.data(), buffer.size());
}

//filtered version for file streamds
auto Parser::parse(std::istream& data, const std::vector<char>& messages) -> std::vector<Message> {
    auto buffer = read_stream_into_buffer(data);
    return parse(buffer.data(), buffer.size(), messages);
}
}





