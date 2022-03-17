#include "client_socket.hpp"

using namespace std;

int get_client_socket(char* host, int port) {
    int cli_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (cli_sock < 0) {
        cerr << "ERROR while creating a socket" << endl;
        return -2;
    }

    hostent *server;
    server = gethostbyname(host);
    if (!server) {
        cerr << "ERROR: cannot determine host" << endl;
        return -2;
    }

    // Fill address fields before try to connect
    // memset((char*)&server_addr, 0, sizeof(server_addr));
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (server->h_addr_list[0])
        memcpy((char*)server->h_addr_list[0], (char*)&server_addr.sin_addr, server->h_length);
    else {
        cerr << "ERROR: cannot get address from the host!" << endl;
        return -2;
    }
    if (connect(cli_sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "ERROR: connection cannot be established!" << endl;
        return -2;
    }

    set_nonblock(cli_sock);
    return cli_sock;
}