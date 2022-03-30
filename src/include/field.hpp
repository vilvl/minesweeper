#pragma once

#include "field_cell.hpp"
#include "field_base.hpp"

#include <algorithm>  // min & max
#include <random>
#include <memory>
#include <vector>

using op_vec_ptr = std::unique_ptr<std::vector<OpenedCell>>;

class Field : public FieldBase {
 private:
    std::unique_ptr<std::vector<std::vector<FieldCell>>> cells;

 public:
    uint32_t cells_opened = 0;

 private:
    void init(coords crds);
    void generate_field(coords start_crds);
    void count_neighbors();
    void inc_neighbors(coords crds);

    void check_win_condition();

    FieldCell &get_cell(coords crds);
    cell_condition get_cell_condition(coords crds);
    cell_condition get_true_condition(coords crds);
    void open_cell_recursive(coords crds, op_vec_ptr &op_cells, bool first_iter = false);

    void open_closed_neighbors(coords crds, op_vec_ptr &op_cells);
    void flag_closed_neighbors(coords crds);
    uint8_t count_closed_neighbors(coords crds);
    uint8_t count_flaged_neighbors(coords crds);

    void load_preset(int preset, uint16_t &field_width,
                uint16_t &field_hight, uint32_t &mines_total);

 public:
    Field(uint16_t field_width, uint16_t field_hight, uint32_t mines_total);
    explicit Field(uint8_t preset);
    cell_condition get_sprite(coords cur, bool l_button_is_pressed, coords mouse);
    op_vec_ptr open_cell(coords crds);
    op_vec_ptr open_cells(std::vector<coords> &crds);
    void set_state(field_state st);
    void set_flag(coords crds);
    void set_pause();
};
