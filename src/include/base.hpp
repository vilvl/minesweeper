#pragma once

#include <iostream>

enum class cell_condition {
    E0 = 0,
    E1,
    E2,
    E3,
    E4,
    E5,
    E6,
    E7,
    E8,
    MINE,
    UNDEFINED,
    FLAG,
    WRONG_FLAG,
    EXPLODED,
};

enum class field_state {
    NEWGAME,
    WIN,
    DEFEAT,
    INGAME,
    PAUSE,
    NOTINITED,
};

struct coords {
public:
    int16_t x;
    int16_t y;
    coords(int x, int y): x(x), y(y) {};
    bool operator==(coords& other) const {
        return (x == other.x && y == other.y);
    }
};