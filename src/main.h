#pragma once

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <memory>
#include <utility>

#include <SFML/Graphics.hpp>

struct coords {
public:
    int x;
    int y;
    coords(int x, int y): x(x), y(y) {};
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

class FieldCell;

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
    void generate_field(int mine_probability, coords start_crds);
    void count_neighbors();
    void inc_neighbors(coords crds);

public:
    Field(int field_width, int field_hight, int mine_probability,  coords start_crds);
    ~Field();
    bool check_win_condition();
    void open_field();
    int open_cell(coords crds);
    void set_flag(coords crds);
    cell_condition get_sprite(coords crds);
    int count_closed_neighbors(coords crds);
    int open_closed_neighbors(coords crds);
    int count_flaged_neighbors(coords crds);
};
