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
    ASK_NEW_GAME,   //
    ASK_PAUSE,      //
    SET_READY,      //
    SET_NAME,       // + name
    OPEN_CELL,      // + coords
    FLAG_CELL,      // + coords
};

enum class srv_msg {
    GAME_STATE,     // state
    GAME_FIELD,     // state
    NEW_GAME,       // + field_width + field_hight + mine_counter
    PAUSE,          //
    PLAYERS,        // + number + names
    ITER,           // + field_state
    END_WIN,        // + field_state
    END_DEFEAT,     // + field_state
    YOU_HOST,
    NEW_PLAYER,
    PLAYER_DISCONNECTED,
    TEXT_MSG,
};

enum class app_state {
    NOTINITED,
    WAITING_NG,
    INGAME,
    FINISHED,
};
