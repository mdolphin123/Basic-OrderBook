#pragma once
#include "LevelInfo.h"

class OrderBookLevelInfos { //bids_ and asks_ are vectors of LevelInfo structs!
    public: 
        OrderBookLevelInfos(const LevelInfos& bids, const LevelInfos& asks) //constructor!
            : bids_{ bids }
            , asks_{ asks }
        { }
        const LevelInfos& GetBids() const {return bids_; } //getter functions
        const LevelInfos& GetAsks() const {return asks_; }
    private: //actual variables
        LevelInfos bids_;
        LevelInfos asks_;
};