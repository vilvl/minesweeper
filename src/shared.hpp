#pragma once

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <set>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

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

int set_nonblock(int fd);
