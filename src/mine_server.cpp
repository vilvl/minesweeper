#include "include/shared.hpp"
#include "include/field.hpp"

using namespace std;

using cli_sock_ptr = unique_ptr<sf::TcpSocket> ;
using players_map = map<cli_sock_ptr, string>;


class ServerApp {
private:
    bool unblock_sockets;
    void accept_new_connection();
    void send_all(sf::Packet &pack);
    void handle_client_data(const cli_sock_ptr &cli_sock, sf::Packet pack);
    void init_new_game(uint16_t field_width, uint16_t field_hight, uint32_t mines_total);
    void set_name(const cli_sock_ptr &cli_sock, string player_name);
    void set_pause();
    void open_cell(coords crds);
    void flag_cell(coords crds);

public:
    players_map players;
    sf::TcpListener listener;
    sf::SocketSelector selector;
    unique_ptr<Field> field = nullptr;

public:
    ServerApp(u_int port, sf::IpAddress ip = sf::IpAddress::Any,
              bool unblock_sockets = false);
    ~ServerApp() {
        // listener.~Socket();
        // players.~map<>;
    }
    void main_loop();
};


void ServerApp::init_new_game(uint16_t field_width, uint16_t field_hight, uint32_t mines_total) {
    field.reset(new Field(field_width, field_hight, mines_total));
    sf::Packet pack;
    pack << uint8_t(srv::NEW_GAME) << field_width << field_hight << mines_total;
    send_all(pack);
    cout << "New game started" << endl;
}

void ServerApp::set_name(const cli_sock_ptr &cli_sock, string player_name) {
    // players[cli_sock] = player_name;
    sf::Packet pack;
    pack << uint8_t(srv::PLAYERS) << uint16_t(cli_sock->getLocalPort()) << player_name;
    send_all(pack);
    cout << cli_sock->getRemoteAddress().toString() << ":" << cli_sock->getRemotePort()
        << " set name to \"" << player_name << "\"" << endl;
}

void ServerApp::set_pause() {
    field->set_pause();
    sf::Packet pack;
    pack << uint8_t(srv::PAUSE) << (field->state == PAUSE);
    send_all(pack);
    cout << "pause set" << endl;
}

void ServerApp::open_cell(coords crds) {
    field->open_cell(crds);
    sf::Packet pack;
    pack << uint8_t(
            (field->state == INGAME ? srv::ITER :
                (field->state == DEFEAT ? srv::END_DEFEAT : srv::END_WIN)));
    pack << uint32_t(field->ingame_time + field->ingame_time_total);
    for (uint16_t x = 0; x < field->field_width; ++x) {
        for (uint16_t y = 0; y < field->field_hight; ++y) {
            pack << uint8_t(field->get_cell_condition(coords(x,y)));
        }
    }
    send_all(pack);
    cout << "cell opened " << crds.x << ", " << crds.y << endl;
}

void ServerApp::flag_cell(coords crds) {
    field->set_flag(crds);
    sf::Packet pack;
    pack << uint8_t(
            (field->state == INGAME ? srv::ITER :
                (field->state == DEFEAT ? srv::END_DEFEAT : srv::END_WIN)));
    pack << uint32_t(field->ingame_time + field->ingame_time_total);
    for (uint16_t x = 0; x < field->field_width; ++x) {
        for (uint16_t y = 0; y < field->field_hight; ++y) {
            pack << uint8_t(field->get_cell_condition(coords(x,y)));
        }
    }
    send_all(pack);
    cout << "flag set " << crds.x << ", " << crds.y << endl;
}

////////////////////////////////////////////////////////////////////////

void ServerApp::send_all(sf::Packet &pack) {
    for (auto &player : players) {
        const cli_sock_ptr &cli_sock = player.first;
        if (cli_sock->send(pack) != sf::Socket::Done)
            cerr << "Error while sending data" << endl;
    }
}

void ServerApp::handle_client_data(const cli_sock_ptr &cli_sock, sf::Packet pack) {
    int8_t msg_type;
    pack >> msg_type;
    switch(cli::msg_type(msg_type)) {
        case cli::NEW_GAME: {
            uint16_t field_width, field_hight, mines_total;
            pack >> field_width >> field_hight >> mines_total;
            init_new_game(field_width, field_hight, mines_total);
            break;
        } case cli::SET_NAME: {
            string player_name;
            pack >> player_name;
            set_name(cli_sock, player_name);
            break;
        } case cli::PAUSE: {
            set_pause();
            break;
        } case cli::OPEN_CELL: {
            coords crds(0,0);
            pack >> crds;
            open_cell(crds);
            break;
        } case cli::FLAG_CELL: {
            coords crds(0,0);
            pack >> crds;
            flag_cell(crds);
            break;
        }
    }
}

void ServerApp::accept_new_connection() {
    cli_sock_ptr cli_sock(new sf::TcpSocket);
    if (listener.accept(*cli_sock) == sf::Socket::Done) {
        cout << "New connection " << cli_sock->getRemoteAddress().toString() << ":" << cli_sock->getRemotePort() << endl;
        cli_sock->setBlocking(unblock_sockets);
        selector.add(*cli_sock);
        players.insert(make_pair(move(cli_sock), cli_sock->getRemoteAddress().toString()));
    } else {
        cerr << "Error while accepting a connection" << endl;
        cli_sock.reset();
    }
}

void ServerApp::main_loop() {
    while (true) {
        if (selector.wait()) {
            if (selector.isReady(listener)) {
                accept_new_connection();
            }
            for (auto it = players.begin(); it != players.end();) {
                const cli_sock_ptr &cli_sock = it->first;
                if (selector.isReady(*cli_sock)) {
                    sf::Packet pack;
                    switch (cli_sock->receive(pack)) {
                        case sf::Socket::Disconnected: {
                            selector.remove(*cli_sock);
                            it = players.erase(it);
                            cout << "connection closed" << endl;
                            break;
                        } case sf::Socket::Error: {
                            selector.remove(*cli_sock);
                            it = players.erase(it);
                            cerr << "Error while recieving data" << endl;
                            break;
                        } case sf::Socket::Done: {
                            handle_client_data(cli_sock, pack);
                            it++;
                            break;
                        } default: {
                            it++;
                            break;
                        }
                    }
                } else {
                    it++;
                }
            }
        }
    }
}

ServerApp::ServerApp(u_int port, sf::IpAddress ip,
            bool unblock_sockets): unblock_sockets(unblock_sockets) {
    if (listener.listen(port, ip) != sf::Socket::Done) {
        cerr << "ERROR: cant bind master-soclet" << endl;
        // exit(-1);
    } else {
        listener.setBlocking(unblock_sockets);
        selector.add(listener);
        field.reset(new Field(10, 10, 10));
    }
}

/////////////////////////////////////////////////////////////////////////////////////

void parse_args(int argc, char *argv[], u_int &port) {
    if (!(argc == 2 && (port = atoi(argv[1])) && (port > 0) && port <= (MAX_PORT))) {
        cerr << "ERROR: cant get port from cmd argument" << endl;
        exit(-1);
    }
}

int main(int argc, char *argv[]) {
    u_int port;
    // sf::IpAddress ip_address = sf::IpAddress::Any;
    parse_args(argc, argv, port);

    ServerApp app(port);

    app.main_loop();
}
