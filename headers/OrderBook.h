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
        std::thread ordersPruneThread_;
        std::condition_variable shutdownConditionVariable_;
        std::atomic<bool> shutdown_{ false };
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
         
};
