#pragma once

#include <functional>
#include <istream>
#include <map>
#include <vector>

#include "Messages.h"
#include "Indicators.h"

//header file for parser! Defines structure

namespace itch {
    //type alias for callback function
    using MessageCallback = std::function<void(const Message&)>;


    class Parser {
        public:
            Parser(); //public interface for the parser class

            auto parse(const char* data, size_t size, const MessageCallback& callback) -> void;
            auto parse(const char* data, size_t size) -> std::vector<Message>;
            auto parse(const char* data, size_t size, const std::vector<char>& messages) -> std::vector<Message>;
            auto parse(std::istream& data, const MessageCallback& callback) -> void;
            auto parse(std::istream& data) -> std::vector<Message>;
            auto parse(std::istream& data, const std::vector<char>& messages) -> std::vector<Message>;

        private:
            using Handler = std::function<Message(const char*)>; //Type alias for handler
            std::map<char, Handler> m_handlers; //lookup table to map characters to handlers
            
            template <typename T> auto register_handler(char type) -> void; //declaration of register handler
    };
    
    
    namespace utils {
        // Generic byte-swapping function for any integral type
        template <typename T> T swap_bytes(T value) { //template function for any integer type
            static_assert(std::is_integral_v<T>, "swap_bytes can only be used with integral types"); //compile time safety check
            union { //unions with same memory!
                T val;
                uint8_t bytes[sizeof(T)];
            } src, dst;
            src.val = value; //put input value into src
            for (size_t i = 0; i < sizeof(T); ++i) {
                dst.bytes[i] = src.bytes[sizeof(T) - 1 - i];
            }
            return dst.val; //byte swapped val
        }
        
        //declarationsm implemented later
        inline bool is_little_endian();
        template <typename T> T from_big_endian(T value);
        template <typename T> T unpack(const char* buffer, size_t& offset);
        inline void unpack_string(const char* buffer, size_t& offset, char* dest, size_t size);
        inline uint64_t unpack_timestamp(const char* buffer, size_t& offset);
    }

}

        




