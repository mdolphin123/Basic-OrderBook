#pragma once
#include <format>
#include <list>
#include <memory>
#include <stdexcept>
#include "Usings.h"
#include "OrderType.h"
#include "Side.h"
#include "Constants.h"



class Order { //class for one order in the book
    public: 
        Order(OrderType ordertype, OrderId orderId, Side side, Price price, Quantity quantity) //constructor to create a new order!
            :ordertype_{ordertype}
            ,orderId_{orderId}
            ,side_{side}
            ,price_{price}
            ,initialquantity_{quantity}
            ,remainingquantity_{quantity}
        { }

        Order(OrderId orderId, Side side, Quantity quantity) 
            : Order(OrderType::Market, orderId, side, Constants::InvalidPrice, quantity)
        { }
        
        OrderId GetOrderId() const { return orderId_; } //all getters
        Side GetSide() const {return side_; }
        Price GetPrice() const {return price_; }
        OrderType GetOrderType() const {return ordertype_;}
        Quantity GetInitialQuantity() const {return initialquantity_; }
        Quantity GetRemainingQuantity() const {return remainingquantity_; }
        Quantity GetFilledQuantity() const {return GetInitialQuantity()- GetRemainingQuantity(); } //filled quantity!
        bool IsFilled() const { return GetRemainingQuantity() == 0; }  //has the order been filled
        void Fill(Quantity quantity) { //update the remaining quantity
            if(quantity > remainingquantity_) { //cannot fill more than what is left
                throw std::logic_error(std::format("Order ({}) cannot be filled for more than its remaining quantity.", GetOrderId()));
            }
            remainingquantity_ -= quantity;
        }
        void ToGoodTillCancel(Price price) {
            if(GetOrderType() != OrderType::Market) { //we should only be converting market orders
                throw std::logic_error(std::format("Order ({}) cannot have its price adjusted, only market orders can.", GetOrderId()));
            }
            if(!std::isfinite(price)) { //price must be finite!
                throw std::logic_error(std::format("Order ({}) must be a tradeable price", GetOrderId()));
            }
            price_ = price; 
            ordertype_ = OrderType::GoodTillCancel; //changing the ordertype, just changing the field
        }

    private:
        OrderType ordertype_;
        OrderId orderId_;
        Side side_;
        Price price_;
        Quantity initialquantity_;
        Quantity remainingquantity_;
};

//more type aliases
using OrderPointer = std::shared_ptr<Order>; //reference to one order
using OrderPointers = std::list<OrderPointer>; //for orders at one price level (sorted by time)