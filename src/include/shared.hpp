#pragma once

#include <iostream>
#include <memory>
#include <map>

#include "field.hpp"

#include <SFML/Network.hpp>

#define MAX_PORT 65535

inline sf::Packet& operator <<(sf::Packet& packet, const coords& crds) {
    return packet << crds.x << crds.y;
}

inline sf::Packet& operator >>(sf::Packet& packet, coords& crds) {
    return packet >> crds.x >> crds.y;
}

namespace cli {

enum msg_type {
    SET_NAME,   // + name
    NEW_GAME,   //
    PAUSE,      //
    OPEN_CELL,  // + coords
    FLAG_CELL,  // + coords
};

}

namespace srv {

enum msg_type {
    NEW_GAME,       // + field_width + field_hight + mine_counter
    PAUSE,          //
    PLAYERS,        // + number + names
    END_WIN,        // + field_state
    END_DEFEAT,     // + field_state
    ITER,           // + field_state
};

}
