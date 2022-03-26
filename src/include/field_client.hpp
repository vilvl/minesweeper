#pragma once

#include "shared.hpp"

#include <chrono>
#include <unordered_map>

struct PlayerCli {
    uint16_t id;
    uint32_t score = 0;
    std::string name;
    bool its_me = false;
    PlayerCli(uint16_t id, std::string name, bool its_me): id(id), name(name), its_me(its_me) {};
};

struct FieldCli {
private:
    std::vector<std::vector<cell_condition>> cells;
    std::unordered_map<uint16_t, PlayerCli> players;

public:
    uint16_t field_width;
    uint16_t field_hight;
    uint32_t cells_total;
    uint32_t mines_total;
    uint32_t flags_total = 0;
    field_state state = field_state::NEWGAME;
    std::chrono::steady_clock::time_point time_start;
    uint32_t ingame_time = 0;
    uint32_t ingame_time_total = 0;

public:
    FieldCli(uint16_t field_width, uint16_t field_hight, uint32_t mines_total);
    cell_condition get_cell_condition(coords crds);
    bool is_neighbors(coords cur, coords mouse);
    cell_condition get_sprite(coords cur, bool l_button_is_pressed, coords mouse);
    void update(sf::Packet pack);
    void init_players(sf::Packet pack, uint16_t my_id);

    void update_time();
};

