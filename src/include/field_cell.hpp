#pragma once

typedef enum cell_state{
    FLAGGED,
    OPENED,
    CLOSED
} cell_state;

class FieldCell {
private:
public:
    cell_state state = CLOSED;
    bool is_mine;
    int neighbors = 0;

public:
    void set_mine(bool state) {
        this->is_mine = state;
    };
    void set_flag() {
        if (state != OPENED)
            state = (state == FLAGGED ? CLOSED : FLAGGED);
    };
    int open_cell() {
        if (state == OPENED)
            return -1;
        if (state == FLAGGED)
            return -2;
        state = OPENED;
        if (is_mine) {
            return -3;
        } else {
            return neighbors;
        }
    };
};