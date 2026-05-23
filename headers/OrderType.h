#pragma once

enum class OrderType { //order type!
    GoodTillCancel, //ok until executed
    FillandKill, //must be immediately fulfilled or discarded if not possible
    FillorKill,
    GoodForDay,
    Market
};