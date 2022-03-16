#pragma once

#include "field_cell.hpp"

#include <algorithm>  // min & max
#include <ctime>      // time
#include <cstdlib>    // srand


struct coords {
public:
    int x;
    int y;
    coords(int x, int y): x(x), y(y) {};
    bool operator==(coords& other) const {
        return (x == other.x && y == other.y);
    }
};

typedef enum cell_condition {
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
    FLAG
} cell_condition;

class Field {
private:
    FieldCell **cells;

public:
    int field_width;
    int field_hight;
    int mines_total;
    int cells_opened = 0;
    int cells_total;

private:
    FieldCell *get_cell(coords crds);
    void generate_field(coords start_crds);
    void count_neighbors();
    void inc_neighbors(coords crds);

public:
    Field(int field_width, int field_hight, int mines_total);
    ~Field();
    void init(coords crds);
    bool check_win_condition();
    void open_field();
    int open_cell(coords crds);
    void set_flag(coords crds);
    int count_closed_neighbors(coords crds);
    int open_closed_neighbors(coords crds);
    int count_flaged_neighbors(coords crds);
    void flag_closed_neighbors(coords crds);
    cell_condition get_cell_condition(coords crds);
};
