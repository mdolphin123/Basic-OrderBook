#include "../headers/OrderBook.h"
#include "../headers/OrderParse.h"
#include <gtest/gtest.h>

namespace googletest = ::testing;

class OrderBookTestsFixture: public googletest::TestWithParam<const char*> { //gtest class
    private:
        const static inline std::filesystem::path Root { std::filesystem::current_path() }; //current path
        const static inline std::filesystem::path TestFolder{ "gtest" }; 
    public:
        const static inline std::filesystem::path TestFolderPath{ Root / TestFolder }; //fix the path...
};

TEST_P(OrderBookTestsFixture, OrderBookTestSuite) { //creates parametrized test
    //Arrange tests
    const auto file = OrderBookTestsFixture::TestFolderPath / GetParam(); //gets current test parameter

    InputHandler handler; //create the parser object
    const auto [updates, result] = handler.GetInformations(file); //reads the input file and splits into updates and result

    auto GetOrder = [](const Information& information) { //creates order object on heap, returns shared pointer
        return std::make_shared<Order> (
            information.orderType_,
            information.orderId_,
            information.side_,
            information.price_,
            information.quantity_
        );
    };

    auto ModifyOrder = [](const Information& information) { //creates order modify request object
        return OrderModify {
            information.orderId_,
            information.side_,
            information.price_,
            information.quantity_,

        };
    };

    OrderBook orderbook; //actual test execution! Create an orderbook
    for(const auto&update : updates) { //go through every parsed action from the test file
        switch(update.type_) { //what action does the test perform on the orderbook?
            case ActionType::Add: {
                const Trades& trades = orderbook.AddOrder(GetOrder(update));
            }
            break;
            case ActionType::Modify: {
                const Trades& trades = orderbook.ModifyOrder(ModifyOrder(update));
            }
            break;
            case ActionType::Cancel: {
                orderbook.CancelOrder(update.orderId_);
            }
            break;
            default: {
                throw std::logic_error("Unsupported Updates");
            }
        }
    }
    //Final assertions of the test, see if they match up!
    const auto& orderbookInfos = orderbook.GetOrderInfos();
    ASSERT_EQ(orderbook.Size(), result.allCount_);
    ASSERT_EQ(orderbookInfos.GetBids().size(), result.bidCount_);
    ASSERT_EQ(orderbookInfos.GetAsks().size(), result.askCount_);

};

INSTANTIATE_TEST_SUITE_P(Tests, OrderBookTestsFixture, googletest::ValuesIn({
    "Match_GoodTillCancel.txt",
    //"Match_FillandKill.txt",
    //"Match_FillorKill_Hit.txt",
    //"Match_FillorKill_Miss.txt",
    "Cancel_Success.txt",
    //"Modify_Side.txt",
    //"Match_Market.txt"

}));





