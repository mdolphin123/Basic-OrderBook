#include <cstdint>
#include <cstring>
#include <set>
#include <stdexcept>
#include <vector>

#include "itch/messages.hpp"
#include "itch/parser.hpp"   

//use namespace so you can reuse variables, new C++ style
namespace itch {

namespace utils {
    //saves stack overhead, use inline
    //do the endian conversion in preprocessing to save time at runtime, check byte order once!
    inline auto is_little_endian() -> bool {
        #if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            return true
        #elif define(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            return false
        #else
            const union {
                uint32_t i;
                char c[4];
            }
            bint = {0x01020304} //fill the struct, hex notation!
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

        if constexpr(std::is_integral_v<T>> && sizeof(T) > 1) { //if it is a multibyte number, just convert it
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

        //convert from big endian
        high = from_big_endian(high);
        low = from_big_endian(low); 

        return (static_cast<uint64_t>(high) << lower_shift) | low;


    }
} //end of namespace

auto unpack_message(SystemEventMessage& msg, const char* buffer, size_t& offset) -> void {
    msg.event_code = utils::unpack<char>(buffer, offset);
}



}


