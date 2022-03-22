#include <SFML/Graphics.hpp>

#include "include/shared.hpp"
#include "include/field.hpp"
#include "include/button.hpp"
#include "include/textbox.hpp"

#include <cstring>

using namespace std;

int parse_args(int argc, char *argv[], sf::IpAddress &host, u_int &port, string &client_name) {
    if (!(argc == 4
            && (host = argv[1]) != sf::IpAddress::None
            && (port = atoi(argv[2])) && (port > 0) && port <= (MAX_PORT)
            && !(client_name = argv[3]).empty())) {
        std::cerr << "ERROR: cmd arguments should be host-address and port separated with space" << std::endl;
        exit(-1);
    }
}

int main(int argc, char *argv[]) {

    u_int port;
    sf::IpAddress host;
    string client_name;
    parse_args(argc, argv, host, port, client_name);

    sf::TcpSocket socket;
    sf::Socket::Status status = socket.connect(host, port);
    if (status != sf::Socket::Done) {
        cerr << "ERROR: cant bind soclet" << endl;
    }
    // socket.setBlocking(false);

    sf::Packet pack;
    pack << cli::SET_NAME << client_name;

    // TCP socket:
    if (socket.send(pack) != sf::Socket::Done) {
        cerr << "Error: connectoin was not established" << endl;
    }



}
