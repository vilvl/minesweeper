#pragma once

#include "base.hpp"
#include "field_cell.hpp"

#include <algorithm>  // min & max
#include <chrono>
#include <ctime>      // time
#include <cstdlib>    // srand
#include <memory>
#include <vector>

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
    field_state state = field_state::NEWGAME;
    std::chrono::steady_clock::time_point time_start;
    uint32_t ingame_time = 0;
    uint32_t ingame_time_total = 0;

private:
    // FieldCellIterator iterate(coords *crds);
    // void generate_field_probabilisticly(coords start_crds);
    void init(coords crds);
    void generate_field(coords start_crds);
    void count_neighbors();
    void inc_neighbors(coords crds);

    void check_win_condition();
    void set_state(field_state st);

    FieldCell &get_cell(coords crds);
    cell_condition get_cell_condition(coords crds);
    cell_condition get_true_condition(coords crds);
    void open_cell_recursive(coords crds, bool first_iter = false);
    bool is_neighbors(coords current_cell_pos, coords mouse_pos);

    void open_closed_neighbors(coords crds);
    void flag_closed_neighbors(coords crds);
    uint8_t count_closed_neighbors(coords crds);
    uint8_t count_flaged_neighbors(coords crds);

public:
    Field(uint16_t field_width, uint16_t field_hight, uint32_t mines_total);
    ~Field();
    cell_condition get_sprite(coords cur, bool l_button_is_pressed, coords mouse);
    void open_cell(coords crds);
    void set_flag(coords crds);
    void set_pause();
    void upate_time();
};
