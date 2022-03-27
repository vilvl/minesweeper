#include "include/field.hpp"

Field::Field(uint8_t preset):
FieldBase(preset) {
    this->cells.reset(new std::vector<std::vector<FieldCell>>(
                      field_hight, std::vector<FieldCell>(field_width)));
}

Field::Field(uint16_t field_width, uint16_t field_hight, uint32_t mines_total):
FieldBase(field_width, field_hight, mines_total) {
    this->cells.reset(new std::vector<std::vector<FieldCell>>(
                      field_hight, std::vector<FieldCell>(field_width)));
}


FieldCell &Field::get_cell(coords crds) {
    return cells->at(crds.y)[crds.x];
}

void Field::generate_field(coords start_crds) {
    srand(time(NULL));
    int cls = cells_total;
    int mns = mines_total;
    bool is_mn = true;
    if (mns > cls/2) {
        is_mn = false;
        mns = cells_total - mns;
        for (int x = 0; x < field_width; ++x)
            for (int y = 0; y < field_hight; ++y)
                if (!is_neighbors(start_crds, coords(x, y)))
                    get_cell(coords(x, y)).is_mine = true;
    }
    while (mns) {
        coords crds(rand() % field_width, rand() % field_hight);
        if (is_neighbors(start_crds, crds))
            continue;
        FieldCell &cell = get_cell(crds);
        if (cell.is_mine != is_mn) {
            cell.is_mine = is_mn;
            mns--;
        }
    }
    set_state(field_state::INGAME);
}

// void Field::generate_field_probabilisticly(coords start_crds) {
//     int mine_probability = 100 * mines_total / cells_total;
//     this->mines_total = 0;
//     srand(time(NULL));
//     // full field iterator
//     for (int i = 0; i < field_hight; ++i) {
//         for (int j = 0; j < field_width; ++j) {
//             bool mine = false;
//             if (i != start_crds.y || j != start_crds.x)
//                 mine = (rand() % 100 < mine_probability);
//             get_cell(coords(j, i)).set_mine(mine);
//             this->mines_total += mine;
//         }
//     }
// }

void Field::init(coords start_crds) {
    generate_field(start_crds);
    count_neighbors();
}

void Field::inc_neighbors(coords crds) {
    // neighbor iterator
    for (int i = std::max(crds.y - 1, 0); i < std::min(crds.y + 2, field_hight + 0); i++)
        for (int j = std::max(crds.x - 1, 0); j < std::min(crds.x + 2, field_width + 0); j++)
            get_cell(coords(j, i)).neighbors++;
}

void Field::count_neighbors() {
    // full field iterator
    for (int i = 0; i < field_hight; ++i)
        for (int j = 0; j < field_width; ++j)
            if (get_cell(coords(j, i)).is_mine)
                inc_neighbors(coords(j, i));
}

void Field::set_state(field_state st) {
    this->state = st;
    if (state == field_state::WIN || state == field_state::DEFEAT || state == field_state::PAUSE) {
        ingame_time_total += ingame_time;
        ingame_time = 0;
    } else if (state == field_state::INGAME) {
        this->time_start = std::chrono::steady_clock::now();
    }
}

void Field::open_cell_recursive(coords crds, int16_t &score, bool first_iter) {
    int16_t tmp = 0;
    int res = get_cell(crds).open_cell();
    if (res == -3) {
        flags_total += 1;
        score -= 10;
        set_state(field_state::DEFEAT);
        return;
    }
    if (res == -1 && first_iter)
        open_closed_neighbors(crds, score);
    if (res >= 0) {
        this->cells_opened++;
        score += 1;
        if (res == 0) {
            open_closed_neighbors(crds, tmp);
        }
    }
}

void Field::open_cell(coords crds, int16_t &score) {
    if (cells_opened == 0)
        init(crds);
    open_cell_recursive(crds, score, true);
    check_win_condition();
}

void Field::open_closed_neighbors(coords crds, int16_t &score) {
    // neighbor iterator
    if (get_cell(crds).state != cell_state::OPENED
    || get_cell(crds).neighbors != count_flaged_neighbors(crds))
        return;
    for (int i = std::max(crds.y - 1, 0); i < std::min(crds.y + 2, field_hight + 0); i++)
        for (int j = std::max(crds.x - 1, 0); j < std::min(crds.x + 2, field_width + 0); j++)
            open_cell_recursive(coords(j, i), score);
}

uint8_t Field::count_flaged_neighbors(coords crds) {
    // neighbor iterator
    int counter = 0;
    for (int i = std::max(crds.y - 1, 0); i < std::min(crds.y + 2, field_hight + 0); i++)
        for (int j = std::max(crds.x - 1, 0); j < std::min(crds.x + 2, field_width + 0); j++)
            if (get_cell(coords(j, i)).state == cell_state::FLAGGED
            || get_cell(coords(j, i)).state == cell_state::OPENED_MINE)
                counter++;
    return counter;
}

uint8_t Field::count_closed_neighbors(coords crds) {
    // neighbor iterator
    int counter = 0;
     for (int i = std::max(crds.y - 1, 0); i < std::min(crds.y + 2, field_hight + 0); i++)
        for (int j = std::max(crds.x - 1, 0); j < std::min(crds.x + 2, field_width + 0); j++)
            if (get_cell(coords(j, i)).state != cell_state::OPENED
            && get_cell(coords(j, i)).state != cell_state::OPENED_MINE)
                counter++;
    return counter;
}

void Field::flag_closed_neighbors(coords crds) {
    if (count_closed_neighbors(crds) != get_cell(crds).neighbors)
        return;
     for (int i = std::max(crds.y - 1, 0); i < std::min(crds.y + 2, field_hight + 0); i++) {
        for (int j = std::max(crds.x - 1, 0); j < std::min(crds.x + 2, field_width + 0); j++) {
            FieldCell &cell = get_cell(coords(j, i));
            if (cell.state == cell_state::CLOSED) {
                cell.state = cell_state::FLAGGED;
                this->flags_total += 1;
            }
        }
    }
}

void Field::set_pause() {
    if (state == field_state::INGAME) {
        set_state(field_state::PAUSE);
    } else if (state == field_state::PAUSE) {
        set_state(field_state::INGAME);
    }
}

void Field::check_win_condition() {
    if (state == field_state::INGAME && (mines_total + cells_opened == cells_total))
        set_state(field_state::WIN);
}

void Field::set_flag(coords crds) {
    FieldCell &cell = get_cell(crds);
    if (cell.state == cell_state::OPENED) {
        flag_closed_neighbors(crds);
    } else if (cell.state == cell_state::CLOSED) {
        cell.state = cell_state::FLAGGED;
        this->flags_total += 1;
    } else {  // if state == FLAGGED
        cell.state = cell_state::CLOSED;
        this->flags_total -= 1;
    }
}

cell_condition Field::get_cell_condition(coords crds) {
    FieldCell &cell = get_cell(crds);
    switch (cell.state) {
        case cell_state::CLOSED: return cell_condition::UNDEFINED;
        case cell_state::FLAGGED: return cell_condition::FLAG;
        case cell_state::OPENED: return cell_condition(cell.neighbors);
        case cell_state::OPENED_MINE: return cell_condition::EXPLODED;
    }
    return cell_condition::UNDEFINED;
}

cell_condition Field::get_true_condition(coords crds) {
    if (state != field_state::WIN && state != field_state::DEFEAT)
        return get_cell_condition(crds);
    FieldCell &cell = get_cell(crds);
    if (cell.state == cell_state::FLAGGED && !cell.is_mine)
        return cell_condition::WRONG_FLAG;
    if (cell.state == cell_state::OPENED && cell.is_mine)
        return cell_condition::EXPLODED;
    if (cell.state == cell_state::CLOSED && cell.is_mine)
        return cell_condition::MINE;
    return get_cell_condition(crds);
}

cell_condition Field::get_sprite(coords cur, bool l_button_is_pressed, coords mouse) {
    if (state == field_state::WIN || state == field_state::DEFEAT)
        return get_true_condition(cur);

    cell_condition cond = get_cell_condition(cur);
    if (l_button_is_pressed && cond == cell_condition::UNDEFINED
        && ((cur == mouse)
            || (is_neighbors(cur, mouse)
                && get_cell_condition(mouse) >= cell_condition(0)
                && get_cell_condition(mouse) <= cell_condition(8)))) {
        return cell_condition::E0;
    }
    return cond;
}
