#pragma once

#include <iostream>

struct coords {
public:
    int16_t x;
    int16_t y;
    coords(int x, int y): x(x), y(y) {};
    bool operator==(coords& other) const {
        return (x == other.x && y == other.y);
    }
};