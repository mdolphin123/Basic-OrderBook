
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
#include<optional>

#include <numeric>
#include <chrono>
#include <ctime>
#include "OrderBook.h"


//Added background threads on all functions that modify orders_, bids_, or asks_ because there is a background thread!

auto OrderBook::MeasureLatency(LatencyMeasurement& stats, Func&& func) {
    //starts the timer
    auto start = std::chrono::high_resolution_clock::now();
    
    //run the function being timed
    auto result = func();
    
    //stop the timer
    auto end = std::chrono::high_resolution_clock::now();
    
    //elapsed time
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    //record into tracker
    stats.Record(micros);
    
    //return what the function returned
    return result;
}


void OrderBook::PruneGoodForDayOrders()
{    
    //background thread, removes goodforday orders
    using namespace std::chrono;
    const auto end = hours(16);

	while (true) { //independent from main thread, background thread!
		const auto now = system_clock::now(); //get the current timestamp
		const auto now_c = system_clock::to_time_t(now); //duration object representing 16 hours, market close at 4PM
		std::tm now_parts; //creates a tm structure that stores human readable calendar/time fields
		localtime_r(&now_c, &now_parts); //function that stores the current time into that structure

		if (now_parts.tm_hour >= end.count()) {
            now_parts.tm_mday += 1; //moves calendar day forward by 1 if time is after 4PM
        }

         //set target time to market close
		now_parts.tm_hour = end.count();
		now_parts.tm_min = 0;
		now_parts.tm_sec = 0;

		auto next = system_clock::from_time_t(mktime(&now_parts)); //converts back into timestamp
		auto till = next - now + milliseconds(100); //if market is not closed, sleep until it closes! 100ms for safety

        
		{ //create a locked scope! Don't want the main thread to modify orders_ when we are pruning
			std::cout << "in first lock" << std::endl;
            std::unique_lock<std::mutex> ordersLock(ordersMutex_); //acquire the mutex

            bool shouldShutdown = shutdownConditionVariable_.wait_for(ordersLock, till, [this] { //checks why thread woke up
                return shutdown_.load(std::memory_order_acquire); //sees if a shutdown was requested
            }); 
                
            if (shouldShutdown) { //if we need to shutdown, exit the function (kills the background thread)
                return;
            }
		}
        if (shutdown_.load(std::memory_order_acquire)) { //second check for shutdown in case
            return;
        }
        std::cout << "after first lock" << std::endl;

		OrderIds orderIds; //declaring empty list to store IDs of all good for day orders that need to be cancelled

		{ //acquire the mutex again
			std::unique_lock<std::mutex> ordersLock(ordersMutex_); 
            std::cout << "in second lock" << std::endl;

			for (const auto& [id, entry] : orders_) //read from orders!
			{
				const auto& [order, _] = entry;

				if (order->GetOrderType() != OrderType::GoodForDay)
					continue;

				orderIds.push_back(order->GetOrderId()); //add the GoodForDay ids to the list
			}
		}
        std::cout << "after second lock" << std::endl;
		CancelOrders(orderIds); //cancel these orders! Note that we cancel them outside of the scope of the mutex
        //so we dont recursive lock
	}
}

//both of these functions make use of cancelorderinternal, which only cancels one order
void OrderBook::CancelOrders(OrderIds orderIds) { //to cancel multiple orders but only have to lock once
    std::unique_lock<std::mutex> ordersLock(ordersMutex_); //acquire mutex

    for (const auto& orderId: orderIds) { //loop through the orders
        CancelOrderInternal(orderId); //cancel one order at a time
    }

}

//for if a trader cancels a order, helper function with NO lock!
void OrderBook::CancelOrderInternal(OrderId orderId) { 
    if(!orders_.contains(orderId)) {
        return;
    }
    const auto& [order, orderIterator] = orders_.at(orderId); //gets the order and the order Iterator
    
    if(order -> GetSide() == Side::Sell) { //for sell orders
        auto price = order -> GetPrice();
        auto& orders = asks_.at(price); //finds price level in asks
        orders.erase(orderIterator); //gets rid of those prices
        if(orders.empty()) { //if the price level is empty, we remove the ENTIRE price level
            asks_.erase(price);
        }
    }
    if(order -> GetSide() == Side::Buy) { //same thing for buy side
        auto price = order -> GetPrice();
        auto& orders = bids_.at(price);
        orders.erase(orderIterator);
        if(orders.empty()) {
            bids_.erase(price);
        }
    }
    
    OnOrderCancelled(order);
    orders_.erase(orderId); //erases the order from the actual main order map
}

//When an order is cancelled update the level data
void OrderBook::OnOrderCancelled(OrderPointer order) {
    UpdateLevelData(order -> GetPrice(), order -> GetRemainingQuantity(), LevelData::Action::Remove);
}

//general API, on order matched
void OrderBook::OnOrderMatched(Price price, Quantity quantity, bool isFullyFilled) {
    if(isFullyFilled) {
        UpdateLevelData(price, quantity, LevelData::Action::Remove); //remove the order level
    }
    else {
        UpdateLevelData(price, quantity, LevelData::Action::Match); //update the level
    }
}

void OrderBook::UpdateLevelData(Price price, Quantity quantity, LevelData::Action action) {  
    //aggregated data at each price level

    auto& data = data_[price]; //think dict in python! 
    //Retrieves the LevelData object associated with given price

    if (action == LevelData::Action::Remove) { //decrement count
        data.count_ += -1;
    }
    else if (action == LevelData::Action::Add) { //increment count
        data.count_ += 1;
    }


    //count and quantity are not the same, count is for orders and quantity is for shares
    if(action == LevelData::Action::Remove || action == LevelData::Action::Match) { //remove the needed shares
        data.quantity_ -= quantity;
    }
    else { //add the needed shares
        data.quantity_ += quantity;
    }

    if(data.count_ == 0) { //no orders remaining at this price level, we delete it
        data_.erase(price);

    }

}

//For FillorKill, decides if an order can be filled immediately or not
bool OrderBook::CanFullyFill(Side side, Price price, Quantity quantity) const {
    if(!(CanMatch(side, price))) { //no trade possible
        return false;
    }

    std::optional<Price> threshold; //store the best opposing price

    //if buying
    if(side == Side::Buy) { //determines threshold!
        const auto [askPrice, _] = *asks_.begin(); //lowest ask
        threshold = askPrice;
    }
    else {
        const auto [bidPrice, _] = *bids_.begin(); //highest bid
        threshold = bidPrice;
    }

    for(const auto& [levelPrice, levelData] : data_) { //go through all the levls
        if(threshold.has_value() && 
        ((side == Side::Buy && threshold.value() > levelPrice) ||
        (side == Side::Sell && threshold.value() < levelPrice))) {
            continue;
        } //invalid price levels (shouldnt be higher than best bid or lower than best ask!)

        if(quantity <= levelData.quantity_) { //can this level finish the remaining order
            return true;
        }

        quantity -= levelData.quantity_; //remaining shares needed
    }
    return false; //cannot get matched by levels combined

}

//canmatch only tells you if any match, not which ones
bool OrderBook::CanMatch(Side side, Price price) const {
     //after inserting an order, is there at least one opposing order it could trade against immediately?
    if(side == Side::Buy) { //recall that side is an enum class!
        if(asks_.empty()) {
            return false;
        }
        else {
            const auto& [bestAsk, _] = *asks_.begin(); //dereferences the elements, we only care abt the first one!
            //.begin() returns an iterator that we have to dereference
            return price >= bestAsk; //if price is at least best ask, we can match something
        }
    }
    else { //sell side or bids
        if(bids_.empty()) {
            return false;
        }
        else {
            const auto& [bestBid, _] = *bids_.begin();
            return price <= bestBid; //price is at most best bid, we can match something
        }

    }
}


Trades OrderBook::MatchOrders() { //actually do the matches
    Trades trades;
    trades.reserve(orders_.size()); //preallocate empty memory for trades

    while (!bids_.empty() && !asks_.empty()) {  //keep looping while there are buyers and sellers
        auto& [bidPrice, bids] = *bids_.begin(); //best bid
        auto& [askPrice, asks] = *asks_.begin(); //lowest ask

        if (bidPrice < askPrice) { //spread check: if bid is below ask, no matches possible!
            break;
        }

        auto bid = bids.front(); //best bid price
        auto ask = asks.front(); //best ask price

        auto bidId = bid->GetOrderId(); //id of best bid
        auto askId = ask->GetOrderId(); //id of best ask
        auto bidPriceExec = bid->GetPrice(); //price of best bid
        auto askPriceExec = ask->GetPrice(); //price of best bid

        Quantity quantity = std::min( //number of shares to trade (smaller of the two)
            bid->GetRemainingQuantity(),
            ask->GetRemainingQuantity()
        );
        
        //reduce remaining quantity on both orders
        bid->Fill(quantity);
        ask->Fill(quantity);

        //check if either order was done 
        bool bidFilled = bid->IsFilled(); //if successfully filled or not
        bool askFilled = ask->IsFilled();

        //record both sides of the trade (bid and ask)!
        trades.push_back(Trade{
            TradeInfo{bidId, bidPriceExec, quantity},
            TradeInfo{askId, askPriceExec, quantity}
        });

        //notifies the rest of the system an order was matched
        OnOrderMatched(bidPriceExec, quantity, bidFilled);
        OnOrderMatched(askPriceExec, quantity, askFilled);

        //remove the empty price levels
        if (bidFilled) { 
            bids.pop_front();
            orders_.erase(bidId);
        }

        if (askFilled) {
            asks.pop_front();
            orders_.erase(askId);
        }

        //removes the levels if they are filled!
        if (bids.empty()) { //clean up empty price levels!
            bids_.erase(bidPrice);
        }

        if (asks.empty()) {
            asks_.erase(askPrice);
        }
        //after matching finishes handle fillandkill orders that are left (if there are any left cancel them)
    }
    if (!bids_.empty()) {
        auto& [_, bids] = *bids_.begin();
        auto& order = bids.front();
        if(order -> GetOrderType() == OrderType::FillandKill) { //must check if the order is FillandKill
            CancelOrderInternal(order -> GetOrderId()); //remove the leftover
        }
    }
    if(!asks_.empty()) {
        auto& [_, asks] = *asks_.begin();
        auto& order = asks.front();
        if(order -> GetOrderType() == OrderType::FillandKill) {
            CancelOrderInternal(order -> GetOrderId());
        }

    }
    return trades;
}

//when an OrderBook object is created, automatically start the background pruning thread!
OrderBook::OrderBook() : ordersPruneThread_{ [this] { PruneGoodForDayOrders(); } } { }

//destructor! 
OrderBook::~OrderBook()  {
    std::cout << "destructor start\n";
    {
        std::unique_lock<std::mutex> lock(ordersMutex_);  //must hold the mutex while setting the flag as an atomic operation!
        //otherwise could cause race condition + hanging thread
        shutdown_.store(true, std::memory_order_release);
    }
    shutdownConditionVariable_.notify_all();  // notify AFTER releasing lock
    std::cout << "before join\n";
    
    if (ordersPruneThread_.joinable()) { //
        ordersPruneThread_.join();
    }
    std::cout << "after join\n";
    //wait until the thread finishes to continue destruction, otherwise thread will try to access things
}


//FillorKill APIs
void OrderBook::OnOrderAdded(OrderPointer order) { //when an order is added, update the statistics
    UpdateLevelData(order -> GetPrice(), order -> GetInitialQuantity(), LevelData::Action::Add);
}


Trades OrderBook::AddOrder(OrderPointer order) 
{
    std::unique_lock<std::mutex> ordersLock(ordersMutex_); 
    
    if(orders_.contains(order -> GetOrderId())) { //if the order already exists
        return { }; //return empty vector of trades
    }
    //market type
    if(order -> GetOrderType() == OrderType::Market) {
        if(order -> GetSide() == Side::Buy && !asks_.empty()) {
            const auto& [worstAsk, _] = *asks_.rbegin(); //gets the worst (aka the highest) ask
            order -> ToGoodTillCancel(worstAsk);
        }
        else if(order -> GetSide() == Side::Sell && !bids_.empty()) {
            const auto& [worstBid, _] = *bids_.rbegin();
            order -> ToGoodTillCancel(worstBid);
        }
        else { //no trades possible (one side is empty probably, so return empty)
            return { };
        }
    }
    if(order -> GetOrderType() == OrderType::FillandKill && !CanMatch(order -> GetSide(),order -> GetPrice())) {
        return { }; //fill and kill but cannot be matched
    }

    if(order -> GetOrderType() == OrderType::FillorKill && !CanFullyFill(order -> GetSide(), order -> GetPrice(), order -> GetInitialQuantity())) {
        return { };
    }

    OrderPointers::iterator iterator; //stores where inside the linkedlist this order was inserted
    
    if(order -> GetSide() == Side::Buy) { //buy side insertion!
        auto& orders = bids_[order -> GetPrice()];
        orders.push_back(order); //based on time priority
        iterator = std::next(orders.begin(), orders.size() - 1); //move the iterator
    }
    else {
        auto& orders = asks_[order -> GetPrice()]; //sell side, same thing
        orders.push_back(order);
        iterator = std::next(orders.begin(), orders.size()-1);
    }
    orders_.insert({order -> GetOrderId(), OrderEntry{ order, iterator }}); //after handling side put into overall order list
    
    OnOrderAdded(order);
    return MatchOrders();
}



void OrderBook::CancelOrder(OrderId orderId) { //to cancel one order and only have to lock once
    std::unique_lock<std::mutex> ordersLock(ordersMutex_); 
    CancelOrderInternal(orderId);

}


Trades OrderBook::ModifyOrder(OrderModify order) {
    OrderType orderType;
    {
        std::unique_lock<std::mutex> ordersLock(ordersMutex_); 
        if(!orders_.contains(order.GetOrderId())) { //checks if order ID exists 
            return { };
        }
        const auto& [existingOrder, _] = orders_.at(order.GetOrderId()); //retrieves the existing order
        orderType = existingOrder -> GetOrderType();
    }
    CancelOrder(order.GetOrderId()); //cancels the old order
    return AddOrder(order.ToOrderPointer(orderType)); //returns and adds the new order
}

OrderBookLevelInfos OrderBook::GetOrderInfos() const {
    LevelInfos bidInfos, askInfos;
    bidInfos.reserve(orders_.size());
    askInfos.reserve(orders_.size());

    //for each price level, compute the quantity at that price level
    auto CreateLevelInfos = [](Price price, const OrderPointers& orders) {
        return LevelInfo { price, std::accumulate(orders.begin(), orders.end(), (Quantity) 0, 
            [] (std::size_t runningSum, const OrderPointer& order) {
                return runningSum + order -> GetRemainingQuantity(); }) };  
    };
    for (const auto& [price, orders] : bids_) { //summary for bids
        bidInfos.push_back(CreateLevelInfos(price, orders));
    }
    for(const auto& [price, orders]: asks_) { //summary for asks
        askInfos.push_back(CreateLevelInfos(price, orders));
    }
    return OrderBookLevelInfos{ bidInfos, askInfos}; //full summary
}

