#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "shared.hpp"
#include "field_base.hpp"

struct PlayerCli {
    bool active;
    uint16_t id;
    uint32_t score = 0;
    std::string name;
    bool its_me = false;
    PlayerCli(uint16_t id, std::string name, bool its_me): id(id), name(name), its_me(its_me) {}
};

class FieldCli : public FieldBase {
 private:
    std::vector<std::vector<cell_condition>> cells;

    uint8_t count_closed_neighbors(coords crds);
    void flag_closed_neighbors(coords crds);

 public:
    std::unordered_map<uint16_t, PlayerCli> players;

 public:
    FieldCli(uint16_t field_width, uint16_t field_hight, uint32_t mines_total);
    cell_condition get_cell_condition(coords crds);
    cell_condition get_sprite(coords cur, bool l_button_is_pressed, coords mouse);
    void update(sf::Packet pack);
    void init_players(sf::Packet pack, uint16_t my_id);

    void set_flag(coords crds);

    void update_cells(std::vector<OpenedCell>, uint32_t time);
    void field_cells_to_open(coords crds, std::vector<coords> &op_coords);

};
