#pragma once

#include <iostream>
#include <memory>

#include <SFML/Network.hpp>

#define MAX_PORT 65535

enum msg_type {
    DEFAULT,
    NEW_CONNECTION,
    CONNECTION_CLOSED
};

struct msg {
    msg_type type;
    size_t len;
    char * msg;
};
