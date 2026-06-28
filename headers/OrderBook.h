#pragma once
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <stdexcept>
#include <numeric>
#include <memory>
#include <variant>
#include <optional>
#include <tuple>
#include <format>
#include <list>
#include <vector>
#include <map>
#include <cstdint>
#include <cmath>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <chrono>


#include "Usings.h"
#include "OrderType.h"
#include "LevelInfo.h"
#include "Constants.h"
#include "OrderBookLevelInfos.h"
#include "Side.h"
#include "Order.h"
#include "Trade.h"
#include "TradeInfo.h"
#include "OrderModify.h"

class OrderBook {
    private: 
        //For latency measurements!!!
        struct LatencyMeasurement { //helper struct for tracking latency
            std::atomic<uint64_t> count { 0 }; //counts number of operations measured, all counts start at 0
            std::atomic<uint64_t> totalMicros { 0 }; //running total of measured times
            std::atomic<uint64_t> maxMicros { 0 }; //longest operation seen (worst case)

            void Record(uint64_t micros) { //records one micro
                //.load(), compare_exchange_weak, and .fetch_add() are from std::atomic, they load and add to values!
                count.fetch_add(1, std::memory_order_relaxed); //Increments operation counter
                totalMicros.fetch_add(micros, std::memory_order_relaxed); //Adds this measurement to running total

                uint64_t current = maxMicros.load(std::memory_order_relaxed); //reads current max value
                while(micros > current && !maxMicros.compare_exchange_weak(current, micros, std::memory_order_relaxed)) {}
                //updates if needed 
            }

            double AverageMicros() const { //calculates average
                auto c = count.load();
                if(c == 0) {
                    return 0.0;
                }
                else {
                    return (double)totalMicros.load()/c;
                }
            }
        };

        //Tracks order states
        LatencyMeasurement addOrderStats_;
        LatencyMeasurement cancelOrderStats_;
        LatencyMeasurement modifyOrderStats_;

        using Func = std::function<Trades()>; //Type alias!
        auto MeasureLatency(LatencyMeasurement& stats, Func&& func); //timing wrapper declaration

        //________________________________________________________________________________________________________________________
        //made so we can quickly locate the order by imediately finding the order and its location
        struct OrderEntry {
            OrderPointer order_{ nullptr }; //pointer to actual order object
            OrderPointers::iterator location_; //where the order is inside the linkedlist at price level
        };

        //structure that allows us to locate quantity and count
        struct LevelData {
            Quantity quantity_{ };
            Quantity count_{ };

            enum class Action {
                Add,
                Remove,
                Match
            };
        };

        std::unordered_map<Price, LevelData> data_; //map of LevelData
        std::map<Price, OrderPointers, std::greater<Price>> bids_; //3rd thing is the comparator, bids sorted descending
        std::map<Price, OrderPointers, std::less<Price>> asks_; //asks sorted ascending
        std::unordered_map<OrderId, OrderEntry> orders_; //for fast lookup by ID (cancelling or modifying orders!)
        mutable std::mutex ordersMutex_; //declares regular mutex for multithreading
        
        //for the pruning goodforday
        std::condition_variable shutdownConditionVariable_;
        std::atomic<bool> shutdown_{ false };
        std::thread ordersPruneThread_; //Order matters in C++ so you need to declare at the end!
        void PruneGoodForDayOrders();

        void CancelOrders(OrderIds orderIds);
        void CancelOrderInternal(OrderId orderId);

        //FillorKill APIs
        void OnOrderCancelled(OrderPointer order);
        void OnOrderAdded(OrderPointer order);
        void OnOrderMatched(Price price, Quantity quantity, bool isFullyFilled);
        void UpdateLevelData(Price price, Quantity quantity, LevelData::Action action);
        

        //canmatch only tells you if any match, not which ones
        bool CanMatch(Side side, Price price) const;
        //new API for fill or kill
        bool CanFullyFill(Side side, Price price, Quantity quantity) const;
        Trades MatchOrders();

    public:
        OrderBook(); //constructor for orderbook, automatically starts background thread
        OrderBook(const OrderBook&) = delete; //you cannot copy an orderbook object! Dangerous
        void operator = (const OrderBook&) = delete; //disables copy assignment
        OrderBook(OrderBook&&) = delete; //disables move
        void operator = (OrderBook&&) = delete;
        ~OrderBook(); //destructor: when orderbook is destroyed clean it up!
        //for thread/resource cleanup

        Trades AddOrder(OrderPointer order);
        void CancelOrder(OrderId orderId); //for if a trader cancels a order
        Trades ModifyOrder(OrderModify order);

        std::size_t Size() const { return orders_.size(); } //just gets size of order book (# of active orders)
        OrderBookLevelInfos GetOrderInfos() const;

        void PrintLatencyStats() const;  //for printing latency
         
};
