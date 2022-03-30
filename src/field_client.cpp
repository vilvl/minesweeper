#include "include/field_client.hpp"

FieldCli::FieldCli(uint16_t field_width, uint16_t field_hight, uint32_t mines_total):
FieldBase(field_width, field_hight, mines_total) {
    this->cells = std::vector<std::vector<cell_condition>>(
                    field_hight, std::vector<cell_condition>(field_width));
    for (uint16_t x = 0; x < field_hight; x++)
        for (uint16_t y = 0; y < field_hight; y++)
            cells[y][x] = cell_condition::UNDEFINED;
}

cell_condition FieldCli::get_cell_condition(coords crds) {
    return cells[crds.y][crds.x];
}

cell_condition FieldCli::get_sprite(coords cur, bool l_button_is_pressed, coords mouse) {
    cell_condition cond = cells[cur.y][cur.x];
    if ((state == field_state::INGAME || state == field_state::NEWGAME)
    && l_button_is_pressed
    && cond == cell_condition::UNDEFINED
    && ((cur == mouse)
        || (is_neighbors(cur, mouse)
            && get_cell_condition(mouse) >= cell_condition(0)
            && get_cell_condition(mouse) <= cell_condition(8)))) {
        return cell_condition::E0;
    }
    return cond;
}

void FieldCli::update(sf::Packet pack) {
    uint8_t int_field_state;
    pack >> int_field_state;
    this->state = field_state(int_field_state);
    pack >> this->flags_total;
    pack >> this->ingame_time_total;
    this->ingame_time = 0;
    this->time_start = std::chrono::steady_clock::now();
    for (uint16_t x = 0; x < field_width; ++x) {
        for (uint16_t y = 0; y < field_hight; ++y) {
            uint8_t int_cell_state;
            pack >> int_cell_state;
            cells[y][x] = cell_condition(int_cell_state);
        }
    }
    uint8_t player_counter;
    pack >> player_counter;
    for (uint8_t i = 0; i < player_counter; ++i) {
        int16_t id, score;
        bool active;
        pack >> id >> score >> active;
        if (players.count(id) != 0) {
            players.at(id).score = score;
            players.at(id).active = active;
        }
    }
}

void FieldCli::init_players(sf::Packet pack, uint16_t my_id) {
    players.clear();
    uint8_t player_counter;
    pack >> player_counter;
    for (uint8_t i = 0; i < player_counter; ++i) {
        int16_t id;
        std::string name;
        pack >> id >> name;
        players.insert(std::make_pair(id, PlayerCli(id, name, (my_id == id))));
    }
}

uint8_t FieldCli::count_closed_neighbors(coords crds) {
    // neighbor iterator
    int counter = 0;
     for (int i = std::max(crds.y - 1, 0); i < std::min(crds.y + 2, field_hight + 0); i++)
        for (int j = std::max(crds.x - 1, 0); j < std::min(crds.x + 2, field_width + 0); j++)
            if (get_cell_condition(coords(j, i)) == cell_condition::FLAG
            || get_cell_condition(coords(j, i)) == cell_condition::UNDEFINED
            || get_cell_condition(coords(j, i)) == cell_condition::EXPLODED)
                counter++;
    return counter;
}

void FieldCli::flag_closed_neighbors(coords crds) {
    if (count_closed_neighbors(crds) != uint8_t(get_cell_condition(crds))) {
        return;
    }
     for (int i = std::max(crds.y - 1, 0); i < std::min(crds.y + 2, field_hight + 0); i++) {
        for (int j = std::max(crds.x - 1, 0); j < std::min(crds.x + 2, field_width + 0); j++) {
            cell_condition &cond = cells[i][j];
            if (cond == cell_condition::UNDEFINED) {
                cond = cell_condition::FLAG;
                this->flags_total += 1;
            }
        }
    }
}

void FieldCli::set_flag(coords crds) {
    cell_condition &cond = cells[crds.y][crds.x];
     if (cond == cell_condition::UNDEFINED) {
        cond = cell_condition::FLAG;
        this->flags_total += 1;
    } else if (cond == cell_condition::FLAG) {
        cond = cell_condition::UNDEFINED;
        this->flags_total -= 1;
    } else if (uint8_t(cond) <= 8) {
        flag_closed_neighbors(crds);
    }
}


void FieldCli::update_cells(std::vector<OpenedCell> op_cells, uint32_t time) {
    this->ingame_time_total = time;
    this->ingame_time = 0;
    this->time_start = std::chrono::steady_clock::now();
    for (auto cell : op_cells) {
        cells[cell.crds.y][cell.crds.x] = cell.cond;
    }
}


void FieldCli::field_cells_to_open(coords crds, std::vector<coords> &op_crds) {
    cell_condition cond = cells[crds.y][crds.x];
    if (cond == cell_condition::UNDEFINED) {
        op_crds.push_back(crds);
        return;
    } else if (cond >= cell_condition::E0 && cond <= cell_condition::E8) {
        int counter = 0;
        for (int i = std::max(crds.y - 1, 0); i < std::min(crds.y + 2, field_hight + 0); i++)
            for (int j = std::max(crds.x - 1, 0); j < std::min(crds.x + 2, field_width + 0); j++)
                if (cells[i][j] == cell_condition::FLAG
                || cells[i][j] == cell_condition::EXPLODED)
                    counter++;
                else if (cells[i][j] == cell_condition::UNDEFINED)
                    op_crds.push_back(coords(j, i));
        if (counter != size_t(cells[crds.y][crds.x]))
            op_crds.clear();
    }
}

