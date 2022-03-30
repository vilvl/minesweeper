#pragma once

#include "field_base.hpp"

#include <SFML/Network.hpp>

#define MAX_PORT 65535

inline sf::Packet& operator <<(sf::Packet& packet, const coords& crds) {
    return packet << crds.x << crds.y;
}

inline sf::Packet& operator >>(sf::Packet& packet, coords& crds) {
    return packet >> crds.x >> crds.y;
}

inline sf::Packet& operator <<(sf::Packet& packet, OpenedCell& opened_cells) {
    return packet << opened_cells.crds.x << opened_cells.crds.y << uint8_t(opened_cells.cond);
}

inline sf::Packet& operator >>(sf::Packet& packet, OpenedCell& opened_cells) {
        uint8_t tmp;
        packet >> opened_cells.crds.x >> opened_cells.crds.y >> tmp;
        opened_cells.cond = cell_condition(tmp);
        return packet;
}

enum class cli_msg {
    ASK_STATE,      //
    ASK_FIELD,      //
    ASK_NEW_GAME_PRESET,   // + preset
    ASK_NEW_GAME_PARAMS,   // + params
    ASK_PAUSE,      //
    SET_READY,      //
    SET_NAME,       // + name
    OPEN_CELL,      // + coords
    FLAG_CELL,      // + coords
};

enum class srv_msg {
    GAME_STATE,     // app_state
    GAME_NEW,       // + field_width + field_hight + mine_counter
    GAME_STARTED,   // players
    GAME_FIELD,     // field_state
    CELLS_OPENED,   // + player id + score + size + vector<opened_cells> (cond + coord)
    YOU_DEAD,
    GAME_END,        // + win/lost + player id
    // PLAYER_DISCONNECTED, // + id
    TEXT_MSG,       // + msg
};

enum class app_state {
    NOTINITED,
    PAUSE,
    WAITING_NG,
    INGAME,
    FINISHED,
};
