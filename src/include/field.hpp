#pragma once

#include "field_cell.hpp"

#include <algorithm>  // min & max
#include <chrono>
#include <ctime>      // time
#include <cstdlib>    // srand
#include <memory>
#include <vector>

#include <SFML/Network.hpp>

struct coords {
public:
    int16_t x;
    int16_t y;
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
    FLAG,
    WRONG_FLAG,
    EXPLODED
} cell_condition;

enum FieldState {
    NEWGAME,
    WIN,
    DEFEAT,
    INGAME,
    PAUSE
};

class Field {
private:
    std::unique_ptr<std::vector<std::vector<FieldCell>>> cells;

public:
    uint16_t field_width;
    uint16_t field_hight;
    uint32_t mines_total;
    uint32_t cells_total;
    uint32_t cells_opened = 0;
    uint32_t flags_total = 0;
    FieldState state = NEWGAME;
    std::chrono::steady_clock::time_point time_start;
    uint32_t ingame_time = 0;
    uint32_t ingame_time_total = 0;

private:
    FieldCell &get_cell(coords crds);
    void generate_field(coords start_crds);
    void generate_field_probabilisticly(coords start_crds);
    void count_neighbors();
    void inc_neighbors(coords crds);
    void open_cell_recursive(coords crds, bool first_iter = false);
    FieldCell &iterate(coords *crds);

public:
    Field(uint16_t field_width, uint16_t field_hight, uint32_t mines_total);
    ~Field();
    void init(coords crds);
    void check_win_condition();
    void open_cell(coords crds);
    void open_closed_neighbors(coords crds);
    void set_flag(coords crds);
    uint8_t count_closed_neighbors(coords crds);
    uint8_t count_flaged_neighbors(coords crds);
    void flag_closed_neighbors(coords crds);
    cell_condition get_cell_condition(coords crds);
    cell_condition get_true_condition(coords crds);
    bool is_neighbors(coords current_cell_pos, coords mouse_pos);
    void set_pause();
};
