#include "include/mine_server.hpp"

using namespace std;

int parse_args(int argc, char *argv[], sf::IpAddress &host, u_int &port) {
    if (!(argc == 3
            && (host = argv[1]) != sf::IpAddress::None
            && (port = atoi(argv[2])) && (port > 0) && port <= (MAX_PORT))) {
        std::cerr << "ERROR: cmd arguments should be host-address and port separated with space" << std::endl;
        exit(-1);
    }
}

int main(int argc, char *argv[]) {

    u_int port;
    sf::IpAddress host;
    parse_args(argc, argv, host, port);

    sf::TcpSocket socket;
    sf::Socket::Status status = socket.connect(host, port);
    if (status != sf::Socket::Done) {
        cerr << "ERROR: cant bind soclet" << endl;
    }
    socket.setBlocking(false);

    struct msg_new {
        msg_type msg = NEW_CONNECTION;
        char client_name[32] = "client 1";
    } new_con;

    // TCP socket:
    if (socket.send(&new_con, sizeof(new_con)) != sf::Socket::Done) {
        cerr << "Error: connectoin was not established" << endl;
    }


}
