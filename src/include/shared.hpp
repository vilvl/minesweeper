#pragma once

#include "base.hpp"

#include <SFML/Network.hpp>

#define MAX_PORT 65535

inline sf::Packet& operator <<(sf::Packet& packet, const coords& crds) {
    return packet << crds.x << crds.y;
}

inline sf::Packet& operator >>(sf::Packet& packet, coords& crds) {
    return packet >> crds.x >> crds.y;
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
    // GAME_END_WIN,        // + field_state
    // GAME_END_DEFEAT,     // + field_state
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
