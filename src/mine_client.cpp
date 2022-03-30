#include <memory>

#include "include/field_client.hpp"
#include "include/graphic.hpp"
#include "include/shared.hpp"

using namespace sf;

class ClientApp {
 public:
    ClientApp(sf::IpAddress host, unsigned port, std::string client_name,
                std::string font_path, std::string sprites_path, std::string buttons_path);

    std::string client_name = "";
    uint16_t winner_id = 0;
    std::unique_ptr<FieldCli> field;
    std::unique_ptr<Graphic> graph;
    sf::SocketSelector selector;
    sf::TcpSocket sock;
    uint16_t id;

    std::string get_winner_name();

    void main_loop();
    void handle_field_events(Event &event, coords crds);
    void handle_interface_events(Event &event, Vector2f crds);
    void handle_keyboard_event(Event &event);
    void display_score(Vector2f mouse_pos);

    void init_connection(sf::IpAddress host, unsigned port, std::string &name);
    void send_pack(sf::Packet &pack);
    void recv_pack(sf::Packet &pack);

    void ask_state();
    void set_ready();
    void set_name(std::string client_name);
    void ask_field();
    void ask_ng(uint8_t preset = 1);
    void ask_pause();
    void open_cell(coords crds);
    void flag_cell(coords crds);

    void handle_server_data();
    void handle_game_state(sf::Packet &pack);
    void handle_game_new(sf::Packet &pack);
    void handle_game_field(sf::Packet &pack);
    void handle_text_msg(sf::Packet &pack);
    void handle_cells_opened(sf::Packet &pack);
    void handle_game_end(sf::Packet &pack);
    void handle_you_dead();

    void handle_game_started(sf::Packet &pack);
};


//////////////////////////////////////////////////////////////////////////////////
// #pragma region inet

void ClientApp::init_connection(sf::IpAddress host, unsigned port, std::string &name) {
    this->client_name = name;
    sock.setBlocking(true);
    if (sock.connect(host, port) != sf::Socket::Done)
        std::cerr << "ERROR: canott bind soclet" << std::endl;
    selector.add(sock);
    if (!client_name.empty())
        set_name(client_name);
}

void ClientApp::send_pack(sf::Packet &pack) {
    switch (sock.send(pack)) {
        case sf::Socket::Disconnected: {
            std::cerr << "Server disconected" << std::endl;
            exit(-2);
        } case sf::Socket::Error: {
            std::cerr << "Error on packet recieve" << std::endl;
            exit(-2);
        } default: {}
        // sf::sleep(sf::Time(1000000));
    }
}

void ClientApp::recv_pack(sf::Packet &pack) {
    switch (sock.receive(pack)) {
        case sf::Socket::Disconnected: {
            std::cerr << "Server disconected" << std::endl;
            exit(-2);
        } case sf::Socket::Error: {
            std::cerr << "Error on packet recieve" << std::endl;
            exit(-2);
        } default: {}
        // sf::sleep(sf::Time(1000000));
    }
}

// #pragma endregion
////////////////////////////////////////////////////////////////////////////////////////
// #pragma region client

void ClientApp::set_name(std::string client_name) {
    this->client_name = client_name;
    sf::Packet pack;
    pack << uint8_t(cli_msg::SET_NAME) << client_name;
    send_pack(pack);
}

void ClientApp::set_ready() {
    sf::Packet pack;
    pack << uint8_t(cli_msg::SET_READY);
    send_pack(pack);
}

void ClientApp::ask_state() {
    sf::Packet pack;
    pack << uint8_t(cli_msg::ASK_STATE);
    send_pack(pack);
    while (true) {
        selector.wait();
        // always true?
        if (selector.isReady(sock)) {
            // sf::Packet pack_r;
            uint8_t msg, state;
            recv_pack(pack);
            pack >> msg;
            if (srv_msg(msg) != srv_msg::GAME_STATE) {
                pack.clear();
                continue;
            }
            pack >> this->id;
            pack >> state;
            if (app_state(state) == app_state::NOTINITED)
                std::cout << "game not inited. why?" << std::endl;
            else
                handle_game_new(pack);
            break;
        }
    }
}

void ClientApp::ask_field() {
    sf::Packet pack;
    pack << uint8_t(cli_msg::ASK_FIELD);
    send_pack(pack);
}

void ClientApp::ask_ng(uint8_t preset) {
    sf::Packet pack;
    pack << uint8_t(cli_msg::ASK_NEW_GAME_PRESET);
    pack << preset;
    send_pack(pack);
}

void ClientApp::ask_pause() {
    sf::Packet pack;
    pack << uint8_t(cli_msg::ASK_PAUSE);
    send_pack(pack);
}

void ClientApp::open_cell(coords crds) {
    sf::Packet pack;
    std::vector<coords> op_crds;
    field->field_cells_to_open(crds, op_crds);
    uint32_t sz = op_crds.size();
    if (sz > 0) {
        pack << uint8_t(cli_msg::OPEN_CELL) << sz;
        for (auto &op_crd : op_crds) {
            pack << op_crd;
        }
        send_pack(pack);
    }
}

void ClientApp::flag_cell(coords crds) {
    field->set_flag(crds);
    // sf::Packet pack;
    // pack << uint8_t(cli_msg::FLAG_CELL) << crds;
    // send_pack(pack);
}

// #pragma endregion
////////////////////////////////////////////////////////////////////////////////////////////////////
// #pragma region server

void ClientApp::handle_game_state(sf::Packet &pack) {
    uint8_t state;
    uint16_t field_width, field_hight, my_id;
    uint32_t mines_total;
    pack >> my_id >> state >> field_width >> field_hight >> mines_total;
    std::cout << "state recieved (why tho?): " << state
        << "; id: " << my_id
        << "; field: " << field_width << "x" << field_hight
        << "; mines: " << mines_total << std::endl;
}

void ClientApp::handle_game_new(sf::Packet &pack) {
    uint16_t field_width, field_hight;
    uint32_t mines_total;
    pack >> field_width >> field_hight >> mines_total;
    this->graph->init_window(field_width, field_hight);
    this->field.reset(new FieldCli(field_width, field_hight, mines_total));
    set_ready();
    ask_field();
}

void ClientApp::handle_text_msg(sf::Packet &pack) {
    std::string msg;
    pack >> msg;
    std::cout << msg << std::endl;
}

void ClientApp::handle_game_field(sf::Packet &pack) {
    field->update(pack);
}

void ClientApp::handle_cells_opened(sf::Packet &pack) {
    uint16_t id, score;
    uint32_t sz, time;
    std::vector<OpenedCell> op_cells;

    pack >> time >> id >> score >> sz;
    field->players.at(id).score = score;

    for (uint32_t i = 0; i < sz; ++i) {
        OpenedCell el;
        pack >> el;
        op_cells.push_back(el);
    }

    field->update_cells(op_cells, time);
}


void ClientApp::handle_game_started(sf::Packet &pack) {
    field->init_players(pack, id);
    field->state = field_state::INGAME;
}

void ClientApp::handle_you_dead() {
    field->state = field_state::DEFEAT;
}

void ClientApp::handle_game_end(sf::Packet &pack) {
    bool win;
    pack >> win >> this->winner_id;
    field->state = (win ? field_state::WIN : field_state::DEFEAT);
}


void ClientApp::handle_server_data() {
    sf::Packet pack;
    recv_pack(pack);
    uint8_t msg;
    pack >> msg;
    switch (srv_msg(msg)) {
        case srv_msg::GAME_STATE: {
            handle_game_state(pack);
            break;
        } case srv_msg::TEXT_MSG: {
            handle_text_msg(pack);
            break;
        } case srv_msg::GAME_NEW: {
            handle_game_new(pack);
            break;
        } case srv_msg::GAME_FIELD: {
            handle_game_field(pack);
            break;
        } case srv_msg::GAME_STARTED: {
            handle_game_started(pack);
            break;
        } case srv_msg::YOU_DEAD: {
            handle_you_dead();
            break;
        } case srv_msg::CELLS_OPENED: {
            handle_cells_opened(pack);
            break;
        } case srv_msg::GAME_END: {
            handle_game_end(pack);
        }
    }
}

// #pragma endregion
///////////////////////////////////////////////////////////////////////
// #pragma region actions

ClientApp::ClientApp(sf::IpAddress host, unsigned port, std::string client_name,
            std::string font_path, std::string sprites_path, std::string buttons_path) {
    graph.reset(new Graphic(font_path, sprites_path, buttons_path));
    init_connection(host, port, client_name);
}

void ClientApp::handle_field_events(Event &event, coords crds) {
    if (field->state != field_state::INGAME && field->state != field_state::NEWGAME)
        return;
    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Right) {
        flag_cell(crds);
    }
    if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Left) {
        open_cell(crds);
    }
}

void ClientApp::handle_interface_events(Event &event, Vector2f crds) {
    if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Left) {
        if (graph->buttons_spritesE.getGlobalBounds().contains(crds)) {
            ask_ng(1);
        }
        if (graph->buttons_spritesN.getGlobalBounds().contains(crds)) {
            ask_ng(2);
        }
        if (graph->buttons_spritesH.getGlobalBounds().contains(crds)) {
            ask_ng(3);
        }
    }
}

void ClientApp::handle_keyboard_event(Event &event) {
    if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) {
        ask_pause();
    }
}

void ClientApp::display_score(Vector2f mouse_pos) {
    std::string state_text = (field->state == field_state::WIN ?
        field->players.at(winner_id).name + " WIN!": field->get_state_text());
    graph->draw_interface(
            (field->mines_total - field->flags_total),
            (field->ingame_time_total + field->ingame_time),
            state_text,
            mouse_pos);
    unsigned counter = 0;
    for (auto const &it : field->players) {
        auto &player = it.second;
        graph->draw_score(player.name, player.score, counter++, player.its_me, player.active);
    }
}

void ClientApp::main_loop() {
    bool is_inside_field = false;
    while (graph->window.isOpen()) {
        // ask_field();
        while (selector.wait(sf::milliseconds(10)) && selector.isReady(sock))
            handle_server_data();

        Vector2f scaled_mouse_pos =
                graph->window.mapPixelToCoords(Mouse::getPosition(graph->window));
        coords crds(scaled_mouse_pos.x / graph->cell_size,
                (scaled_mouse_pos.y - graph->interface_shift) / graph->cell_size);
        graph->window.clear(Color::White);
        for (int x = 0; x < field->field_width; x++) {
            for (int y = 0; y < field->field_hight; y++) {
                cell_condition sprite = field->get_sprite(coords(x, y),
                    is_inside_field && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left), crds);
                graph->draw_cell(x, y, static_cast<int>(sprite));
            }
        }
        field->update_time();
        display_score(scaled_mouse_pos);
        graph->window.display();

        Event event;
        while (graph->window.pollEvent(event)) {
            if (event.type == Event::Closed)
                graph->window.close();
            if (scaled_mouse_pos.y >= graph->interface_shift
            && scaled_mouse_pos.y <= graph->interface_shift + graph->field_h) {
                handle_field_events(event, crds);
                is_inside_field = true;
            } else {
                handle_interface_events(event, scaled_mouse_pos);
                is_inside_field = false;
            }
            handle_keyboard_event(event);
        }
    }
}

// #pragma endregion
///////////////////////////////////////////////////////////////////////////////////////////////

void parse_args(int argc, char *argv[],
        sf::IpAddress &host, unsigned &port, std::string &client_name) {
    if (!(argc == 4
            && (host = argv[1]) != sf::IpAddress::None
            && (port = atoi(argv[2])) && (port > 0) && port <= (MAX_PORT)
            && !(client_name = argv[3]).empty())) {
        std::cerr << "ERROR: cmd arguments should be host-address, "
                << "port and client-name separated with spaces" << std::endl;
        exit(-1);
    }
}

int main(int argc, char *argv[]) {
    unsigned port;
    sf::IpAddress host;
    std::string client_name;
    parse_args(argc, argv, host, port, client_name);

    ClientApp app(host, port, client_name,
        "resources/arial.ttf", "resources/heb_tiles.png", "resources/buttons.png");
    app.ask_state();
    app.main_loop();
}
