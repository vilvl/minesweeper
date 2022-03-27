#pragma once


enum class cell_state{
    FLAGGED,
    OPENED,
    CLOSED,
    OPENED_MINE
};


class FieldCell {
 public:
    cell_state state = cell_state::CLOSED;
    bool is_mine;
    int neighbors = 0;

 public:
    void set_mine(bool state) {
        this->is_mine = state;
    }
    void set_flag() {
        if (state != cell_state::OPENED)
            state = (state == cell_state::FLAGGED ? cell_state::CLOSED : cell_state::FLAGGED);
    }
    int open_cell() {
        if (state == cell_state::OPENED || state == cell_state::OPENED_MINE)
            return -1;
        if (state == cell_state::FLAGGED)
            return -2;
        if (is_mine) {
            state = cell_state::OPENED_MINE;
            return -3;
        } else {
            state = cell_state::OPENED;
            return neighbors;
        }
    }
};
