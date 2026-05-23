#pragma once
#include "Usings.h"
#include "Side.h"
#include "TradeInfo.h"
#include <vector>

class Trade { //Trade class to handle trades, represents ONE trade between buyer and seller!
    public: 
        Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade)
            : bidTrade_{ bidTrade }
            , askTrade_{ askTrade }
        {}

        const TradeInfo& GetBidTrade() const { return bidTrade_; }
        const TradeInfo& GetAskTrade() const { return askTrade_; }
    private:
    //bidTrade_ is the buyer's side and askTrade_ is the seller's side! The trade should have same price + quantity
        TradeInfo bidTrade_;
        TradeInfo askTrade_;      
};

using Trades = std::vector<Trade>; //return a vector of trades bc one order can generate a bunch of possible trades
//later functions need to return a list of matched trades