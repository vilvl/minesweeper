#include "include/shared.hpp"
#include "include/field.hpp"
// #include "include/button.hpp"
// #include "include/textbox.hpp"

#include <cstring>
#include <memory>

using namespace std;


class CliField {
private:
    std::unique_ptr<std::vector<std::vector<cell_condition>>> cells;

public:
    int field_width;
    int field_hight;
    int mines_total;
    int flags_total = 0;
    FieldState state = NEWGAME;
    u_int ingame_time_total = 0;

public:
    CliField(int field_width, int field_hight, int mines_total);
    ~CliField() {};
    void open_cell(coords crds);
    void set_flag(coords crds);
    cell_condition &get_cell(coords crds);
};

class ClientApp {
public:
    string client_name = "";
    unique_ptr<sf::TcpSocket> sock;
    unique_ptr<sf::RenderWindow> app = nullptr;
    unique_ptr<CliField> field = nullptr;

    void init_connection(sf::IpAddress host, uint port, string &name) {
        this->client_name = name;
        sock.reset(new sf::TcpSocket);
        sock->setBlocking(true);
        if (sock->connect(host, port) != sf::Socket::Done)
            cerr << "ERROR: cant bind soclet" << endl;
        if (!client_name.empty())
            set_name(client_name);
    }
    void send_pack(sf::Packet &pack) {
        if (sock->send(pack) != sf::Socket::Done) {
            cerr << "Error: connectoin was not established" << endl;
        }
    }
    void recv_pack(sf::Packet &pack) {
        if (sock->receive(pack) != sf::Socket::Done) {
            cerr << "Error: connectoin was not established" << endl;
        }
    }
    void set_name(string client_name) {
        this->client_name = client_name;
        sf::Packet pack;
        cout << client_name << endl;
        pack << cli::SET_NAME << client_name;
        send_pack(pack);
    }
    srv::app_state ask_field_state() {
        sf::Packet pack;
        pack << uint8_t(cli::ASK_STATE);
        send_pack(pack);
        pack.clear();
        recv_pack(pack);
        uint8_t state;
        pack << state;
        return srv::app_state(state);
    }

};



void ClientApp::main_loop() {

    ask_field_state();
    display();
    handle_event();

}

void parse_args(int argc, char *argv[], sf::IpAddress &host, u_int &port, string &client_name) {
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

    ClientApp app;
    app.init_connection(host, port, client_name);
    srv::app_state state = app.ask_state();
    if (state == srv::NOTINITED) {
        app.ask_new_game(10, 10, 10);
        app.ask_field();
        app.init();
        app.set_ready();
    } else if (state == srv::WAITING_NG) {
        app.ask_field();
        app.init();
        app.set_ready();
    }
    app.main_loop();
}
