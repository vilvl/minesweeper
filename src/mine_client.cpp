#include "include/shared.hpp"
#include "include/graphic.hpp"
#include "include/field_client.hpp"

#include <cstring>
#include <memory>

using namespace sf;

class ClientApp {
public:
    ClientApp(sf::IpAddress host, u_int port, std::string client_name,
                std::string font_path, std::string sprites_path);

    std::string client_name = "";
    sf::SocketSelector selector;
    sf::TcpSocket sock;
    std::unique_ptr<FieldCli> field;
    std::unique_ptr<Graphic> graph;
    uint16_t id;

    void main_loop();
    void handle_field_events(Event &event, coords crds);
    void handle_interface_events(Event &event, Vector2f crds);
    void handle_keyboard_event(Event &event);
    void display_score();

    void init_connection(sf::IpAddress host, uint port, std::string &name);
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
    void handle_game_state(sf::Packet &pack);     // state
    void handle_game_new(sf::Packet &pack);       // + field_width + field_hight + mine_counter
    void handle_game_field(sf::Packet &pack);     // field
    void handle_text_msg(sf::Packet &pack);
    void handle_game_started(sf::Packet &pack);
};


//////////////////////////////////////////////////////////////////////////////////////////
#pragma region inet

void ClientApp::init_connection(sf::IpAddress host, uint port, std::string &name) {
    this->client_name = name;
    sock.setBlocking(true);
    if (sock.connect(host, port) != sf::Socket::Done)
        std::cerr << "ERROR: canott bind soclet" << std::endl;
    selector.add(sock);
    if (!client_name.empty())
        set_name(client_name);
}

void ClientApp::send_pack(sf::Packet &pack) {
    if (sock.send(pack) != sf::Socket::Done) {
        std::cerr << "Error: connectoin was not established" << std::endl;
        // sf::sleep(sf::Time(1000000));
    }
}

void ClientApp::recv_pack(sf::Packet &pack) {
    if (sock.receive(pack) != sf::Socket::Done) {
        std::cerr << "Error: connectoin was not established" << std::endl;
        // sf::sleep(sf::Time(1000000));
    }
}

#pragma endregion
/////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region client

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
    pack << uint8_t(cli_msg::OPEN_CELL) << crds;
    send_pack(pack);
}

void ClientApp::flag_cell(coords crds) {
    sf::Packet pack;
    pack << uint8_t(cli_msg::FLAG_CELL) << crds;
    send_pack(pack);
}

#pragma endregion
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region server

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
}

void ClientApp::handle_text_msg(sf::Packet &pack) {
    std::string msg;
    pack >> msg;
    std::cout << msg << std::endl;
}

void ClientApp::handle_game_field(sf::Packet &pack) {
    field->update(pack);
}

void ClientApp::handle_game_started(sf::Packet &pack){
    field->init_players(pack, id);
}

void ClientApp::handle_server_data() {
    sf::Packet pack;
    recv_pack(pack);
    uint8_t msg;
    pack >> msg;
    switch(srv_msg(msg)) {
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
        }
    }
}

#pragma endregion
///////////////////////////////////////////////////////////////////////
#pragma region actions

ClientApp::ClientApp(sf::IpAddress host, u_int port, std::string client_name,
            std::string font_path, std::string sprites_path) {
    graph.reset(new Graphic(font_path, sprites_path));
    init_connection(host, port, client_name);
}

void ClientApp::handle_field_events(Event &event, coords crds) {
    if (field->state != field_state::INGAME && field->state != field_state::NEWGAME)
        return;
    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left)
        graph->buttons_state.l = true;
    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Right) {
        graph->buttons_state.r = true;
        flag_cell(crds);
    }
    if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Left) {
        graph->buttons_state.l = false;
        open_cell(crds);
    }
    if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Right) {
        graph->buttons_state.r = false;
    }
}

void ClientApp::handle_interface_events(Event &event, Vector2f crds) {
    if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Left) {
        ask_ng();
    }
}

void ClientApp::handle_keyboard_event(Event &event) {
    if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) {
        ask_pause();
    }
}
void ClientApp::display_score() {
    std::string state_text;
    switch(field->state) {
        case field_state::WIN:       state_text = "GRAZ";           break;
        case field_state::DEFEAT:    state_text = "YOU LOST";       break;
        case field_state::PAUSE:     state_text = "PAUSE";          break;
        case field_state::NEWGAME:   state_text = "CLICK TO START"; break;
        case field_state::INGAME:    state_text = "";               break;
        default:        state_text = "WTF ITS DISPLAYED???";
    }
    graph->draw_interface(
            std::to_string(field->mines_total - field->flags_total),
            std::to_string(field->ingame_time_total + field->ingame_time),
            state_text
    );
    uint counter = 0;
    for (auto const &it : field->players) {
        auto &player = it.second;
        graph->draw_score(player.name, player.score, counter++, player.its_me, player.active);
    }
}

void ClientApp::main_loop() {
    while (graph->window.isOpen()) {

        ask_field();
        while (selector.wait(sf::milliseconds(10)) && selector.isReady(sock))
            handle_server_data();

        Vector2f scaled_mouse_pos = graph->window.mapPixelToCoords(Mouse::getPosition(graph->window));
        coords crds(scaled_mouse_pos.x / graph->cell_size, (scaled_mouse_pos.y - graph->interface_shift) / graph->cell_size);
        graph->window.clear(Color::White);
		for (int x = 0; x < field->field_width; x++) {
			for (int y = 0; y < field->field_hight; y++) {
                graph->draw_cell(x, y, int(field->get_sprite(coords(x, y), graph->buttons_state.l, crds)));
			}
        }
        field->update_time();
        display_score();
		graph->window.display();

        Event event;
        while (graph->window.pollEvent(event)) {
            if (event.type == Event::Closed)
                graph->window.close();
            if (scaled_mouse_pos.y >= graph->interface_shift
            && scaled_mouse_pos.y <= graph->interface_shift + graph->field_h) {
                handle_field_events(event, crds);
            } else {
                handle_interface_events(event, scaled_mouse_pos);
            }
            handle_keyboard_event(event);
        }
    }
}

#pragma endregion
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void parse_args(int argc, char *argv[], sf::IpAddress &host, u_int &port, std::string &client_name) {
    if (!(argc == 4
            && (host = argv[1]) != sf::IpAddress::None
            && (port = atoi(argv[2])) && (port > 0) && port <= (MAX_PORT)
            && !(client_name = argv[3]).empty())) {
        std::cerr << "ERROR: cmd arguments should be host-address, port and client-name separated with spaces" << std::endl;
        exit(-1);
    }
}

int main(int argc, char *argv[]) {

    u_int port;
    sf::IpAddress host;
    std::string client_name;
    parse_args(argc, argv, host, port, client_name);

    ClientApp app(host, port, client_name,
        "resources/arial.ttf", "resources/heb_tiles.jpg");
    app.ask_state();
    app.main_loop();
}
