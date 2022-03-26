#include "include/shared.hpp"
#include "include/field.hpp"


using namespace std;

struct Player {
    static int16_t curr_id;
    bool active;
    bool host = false;
    bool ready = false;
    int16_t score = 0;
    int16_t id = 0;
    string name = "";
    string address = "";
    unique_ptr<sf::TcpSocket> cli_sock = unique_ptr<sf::TcpSocket>(new sf::TcpSocket);
    Player() {
        id = curr_id++;
    }
};

int16_t Player::curr_id = 0;

inline sf::Packet& operator<<(sf::Packet &pack, vector<Player> &players) {
    pack << uint8_t(players.size());
    for (auto &player : players) {
        pack << player.id << player.score;  // << player.name;
    }
    return pack;
}

class ServerApp {
private:
    bool unblock_sockets;
    void send_all(sf::Packet &pack);
    void send_to_player(Player &player, sf::Packet &pack);
    void handle_new_connection();
    void handle_client_data(Player &player, sf::Packet &pack);

    bool check_players_ready();

    void send_state(Player &player);
    void send_field(Player &player);
    void set_ready(Player &player);
    void set_name(Player &player, string &player_name);
    void set_pause();
    void open_cell(coords crds);
    void flag_cell(coords crds);
    void load_preset(int preset, uint16_t &field_width, uint16_t &field_hight, uint32_t &mines_total);

public:
    vector<Player> players;
    sf::TcpListener listener;
    sf::SocketSelector selector;
    unique_ptr<Field> field = nullptr;
    app_state state = app_state::NOTINITED;

public:
    ServerApp(u_int port, sf::IpAddress ip = sf::IpAddress::Any,
              bool unblock_sockets = false);
    ~ServerApp() {
        // listener.~Socket();
        // players.~map<>;
    }
    void init_new_game(uint16_t field_width, uint16_t field_hight, uint32_t mines_total);
    void init_new_game(uint8_t preset);
    void main_loop();
};

///////////////////////////////////////

bool ServerApp::check_players_ready() {
    for (auto &player : players) {
        if (!player.ready)
            return false;
    }
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////

void ServerApp::send_state(Player &player) {
    sf::Packet pack;
    pack << uint8_t(srv_msg::GAME_STATE) << uint16_t(player.id) << uint8_t(this->state);
    if (field)
        pack << field->field_width << field->field_hight << field->mines_total;
    else
        pack << uint16_t(0) << uint16_t(0) << uint32_t(0);
    send_to_player(player, pack);
    cout << "State sent" << endl;
}

void ServerApp::set_ready(Player &player) {
    if (state != app_state::NOTINITED)
        player.ready = true;
    if (state == app_state::WAITING_NG)
        player.active = true;
    cout << "Ready set" << endl;
    // sf::Packet pack;
    // pack << "player " << player.name << " is ready";
    // send_all(pack);
}

void ServerApp::set_name(Player &player, string &player_name) {
    player.name = player_name;
    cout << player.id << " set name to \"" << player_name << "\"" << endl;
}

void ServerApp::set_pause() {
    if (state == app_state::INGAME) {
        state = app_state::PAUSE;
        field->set_pause();
    } else if (state == app_state::PAUSE) {
        state = app_state::PAUSE;
        field->set_pause();
    }
    cout << "pause set" << endl;
}

void ServerApp::open_cell(coords crds) {
    if (state == app_state::WAITING_NG && check_players_ready()) {
        state = app_state::INGAME;
        sf::Packet pack;
        pack << uint8_t(srv_msg::GAME_STARTED) << uint8_t(players.size());
        for (auto &player : players)
            pack << player.id << player.name;
        send_all(pack);
    } if (state != app_state::INGAME) {
        return;
    }
    field->open_cell(crds);
    if (field->state == field_state::WIN || field->state == field_state::DEFEAT)
        state = app_state::FINISHED;
    cout << "cell opened: " << crds.x << ", " << crds.y << endl;
}

void ServerApp::flag_cell(coords crds) {
    if (state != app_state::INGAME)
        return;
    field->set_flag(crds);
    cout << "flag set: " << crds.x << ", " << crds.y << endl;
}

void ServerApp::send_field(Player &player) {
    // msg_type << field_state << time << field << score
    field->upate_time();
    sf::Packet pack;
    pack << uint8_t(srv_msg::GAME_FIELD);
    pack << uint8_t(field->state);
    pack << uint32_t(field->flags_total);
    pack << uint32_t(field->ingame_time_total + field->ingame_time);
    for (uint16_t x = 0; x < field->field_width; ++x) {
        for (uint16_t y = 0; y < field->field_hight; ++y) {
            pack << uint8_t(field->get_sprite(coords(x,y), false, coords(0,0)));
        }
    }
    pack << players;
    send_to_player(player, pack);
    // cout << "field sent" << endl;
}

////////////////////////////////////////////////////////////////////////

void ServerApp::handle_client_data(Player &player, sf::Packet &pack) {
    uint8_t msg_type;
    pack >> msg_type;
    switch(cli_msg(msg_type)) {
        case cli_msg::ASK_STATE: {
            send_state(player);
            break;
        } case cli_msg::ASK_FIELD: {
            send_field(player);
            break;
        } case cli_msg::ASK_NEW_GAME_PRESET: {
            uint8_t preset;
            pack >> preset;
            init_new_game(preset);
            break;
        } case cli_msg::ASK_NEW_GAME_PARAMS: {
            uint16_t field_width, field_hight, mines_total;
            pack >> field_width >> field_hight >> mines_total;
            init_new_game(field_width, field_hight, mines_total);
            break;
        } case cli_msg::SET_NAME: {
            string player_name;
            pack >> player_name;
            cout << player_name << endl;
            set_name(player, player_name);
            break;
        } case cli_msg::SET_READY: {
            set_ready(player);
            break;
        } case cli_msg::ASK_PAUSE: {
            set_pause();
            break;
        } case cli_msg::OPEN_CELL: {
            coords crds(0,0);
            pack >> crds;
            open_cell(crds);
            break;
        } case cli_msg::FLAG_CELL: {
            coords crds(0,0);
            pack >> crds;
            flag_cell(crds);
            break;
        }
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////

void ServerApp::handle_new_connection() {
    players.emplace_back();
    Player &player = players.back();
    if (listener.accept(*player.cli_sock) == sf::Socket::Done) {
        player.address = player.cli_sock->getRemoteAddress().toString() + ":" + to_string(player.cli_sock->getRemotePort());
        player.cli_sock->setBlocking(unblock_sockets);
        selector.add(*player.cli_sock);
        cout << "New connection: " << player.address << endl;
    } else {
        players.pop_back();
        cerr << "Error while accepting a connection" << endl;
    }
}

void ServerApp::send_all(sf::Packet &pack) {
    for (auto &player : players) {
        if (player.cli_sock->send(pack) != sf::Socket::Done)
            cerr << "Error while sending data" << endl;
    }
}

void ServerApp::send_to_player(Player &player, sf::Packet &pack) {
    if (player.cli_sock->send(pack) != sf::Socket::Done)
        cerr << "Error while sending data" << endl;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void ServerApp::load_preset(int preset, uint16_t &field_width, uint16_t &field_hight, uint32_t &mines_total) {
    switch (preset) {
        case 1: {
            field_width = 9;
            field_hight = 9;
            mines_total = 10;
            break;
        }
        case 2: {
            field_width = 16;
            field_hight = 16;
            mines_total = 40;
            break;
        }
        case 3:
            field_width = 30;
            field_hight = 16;
            mines_total = 99;
            break;
    }
}

void ServerApp::init_new_game(uint16_t field_width, uint16_t field_hight, uint32_t mines_total) {
    field.reset(new Field(field_width, field_hight, mines_total));
    for (auto &player : players) {
        player.active = false;
        player.ready = false;
    }
    this->state = app_state::WAITING_NG;
    sf::Packet pack;
    pack << uint8_t(srv_msg::GAME_NEW) << field_width << field_hight << mines_total;
    send_all(pack);
    cout << "New game inited" << endl;
}

void ServerApp::init_new_game(uint8_t preset) {
    uint16_t field_width, field_hight;
    uint32_t mines_total;
    load_preset(preset, field_width, field_hight, mines_total);
    init_new_game(field_width, field_hight, mines_total);
}

void ServerApp::main_loop() {
    while (true) {
        if (selector.wait()) {
            if (selector.isReady(listener)) {
                handle_new_connection();
            }
            for (auto it = players.begin(); it != players.end();) {
                if (selector.isReady(*it->cli_sock)) {
                    sf::Packet pack;
                    switch (it->cli_sock->receive(pack)) {
                        case sf::Socket::Disconnected: {
                            selector.remove(*it->cli_sock);
                            it = players.erase(it);
                            cout << "connection closed" << endl;
                            break;
                        } case sf::Socket::Error: {
                            selector.remove(*it->cli_sock);
                            it = players.erase(it);
                            cerr << "Error while recieving data" << endl;
                            break;
                        } case sf::Socket::Done: {
                            handle_client_data(*it, pack);
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
        exit(-1);
    } else {
        listener.setBlocking(unblock_sockets);
        selector.add(listener);
        // field.reset(new Field(10, 10, 10));
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
    // NOTINITED is not possible for client
    app.init_new_game(2);
    app.main_loop();
}
