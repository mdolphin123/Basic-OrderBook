#pragma once
#include "Constants.h"
#include "Usings.h"
#include "Side.h"
#include "Order.h"

#include <memory>
#include <format>
#include <list>


class OrderModify { //requests to modify existing order
    public: 
        OrderModify(OrderId orderId, Side side, Price price, Quantity quantity) 
            : orderId_{ orderId }
            , side_{ side }
            , price_{ price }
            , quantity_{ quantity }
        { }

        OrderId GetOrderId() const { return orderId_; }
        Price GetPrice() const {return price_; }
        Side GetSide() const {return side_; }
        Quantity GetQuantity() const {return quantity_; }

        OrderPointer ToOrderPointer(OrderType type) const { 
            //creates new order object and hands it off to the rest of the system
            return std::make_shared<Order>(type, GetOrderId(), GetSide(), GetPrice(), GetQuantity());
            //basically construct a new order object in memory and return a shared_ptr pointing to it!
        }
    private: 
        OrderId orderId_;
        Price price_;
        Side side_;
        Quantity quantity_;
};
