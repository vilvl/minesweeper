#include "include/field_client.hpp"

FieldCli::FieldCli(uint16_t field_width, uint16_t field_hight, uint32_t mines_total):
FieldBase(field_width, field_hight, mines_total) {
    this->cells = std::vector<std::vector<cell_condition>>(
                    field_hight, std::vector<cell_condition>(field_width));
}

cell_condition FieldCli::get_cell_condition(coords crds) {
    return cells[crds.y][crds.x];
}

cell_condition FieldCli::get_sprite(coords cur, bool l_button_is_pressed, coords mouse) {
    cell_condition cond = cells[cur.y][cur.x];
    if (state == field_state::INGAME
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
    for (uint8_t i = 0; i < player_counter; ++i){
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
    for (uint8_t i = 0; i < player_counter; ++i){
        int16_t id;
        std::string name;
        pack >> id >> name;
        players.insert(std::make_pair(id, PlayerCli(id, name, (my_id == id))));
    }
}
