#include "include/mine_server.hpp"

using namespace std;
typedef unique_ptr<sf::TcpSocket> cli_sock_ptr;
typedef map<cli_sock_ptr, string> players_map;

void parse_args(int argc, char *argv[], u_int &port) {
    if (!(argc == 2 && (port = atoi(argv[1])) && (port > 0) && port <= (MAX_PORT))) {
        cerr << "ERROR: cant get port from cmd argument" << endl;
        exit(-1);
    }
}

void send_all(players_map players, sf::Packet &pack) {
    for (auto &player : players) {
        const cli_sock_ptr &cli_sock = player.first;
        cli_sock->send(pack);
    }
}

void handle_new_connection(const cli_sock_ptr &cli_sock, players_map players) {

}

int handle_client_data(const cli_sock_ptr &cli_sock, players_map players) {
    sf::Packet pack;
    cli_sock->receive(pack);
}

int main(int argc, char *argv[]) {

    u_int port;
    sf::IpAddress ip_address = sf::IpAddress::Any;
    parse_args(argc, argv, port);

    sf::SocketSelector selector;
    sf::TcpListener listener;

    // bind the listener to a port
    if (listener.listen(port) != sf::Socket::Done) {
        cerr << "ERROR: cant bind master-soclet" << endl;
        exit(-1);
    }
    listener.setBlocking(false);
    selector.add(listener);

    players_map players;
    // players_map watchers;

    unique_ptr<Field> field(new Field(10, 0, 0));

    while (true) {
        if (selector.wait()) {
            if (selector.isReady(listener)) {
                cli_sock_ptr cli_sock(new sf::TcpSocket);
                if (listener.accept(*cli_sock) == sf::Socket::Done) {
                    cli_sock->setBlocking(false);
                    selector.add(*cli_sock);
                    players.emplace(cli_sock, cli_sock->getRemoteAddress().toString());
                    handle_new_connection(cli_sock, players);
                } else {
                    cout << "Error while accepting a connection" << endl;
                    cli_sock.reset();
                }
            }
            for (auto it = players.begin(); it != players.end(); it++) {
                const cli_sock_ptr &cli_sock = it->first;
                if (selector.isReady( *cli_sock )) {
                    if (handle_client_data(cli_sock, players) == -1) {
                        selector.remove(*cli_sock);
                        it = players.erase(it);
                    }
                }
            }
        }
    }

}
