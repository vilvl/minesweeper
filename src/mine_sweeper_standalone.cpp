#include "include/field.hpp"
#include "include/graphic.hpp"

#include <memory>

using namespace sf;

class StandAloneApp {
private:
    std::unique_ptr<Field> field;
    std::unique_ptr<Graphic> graph;
    int16_t score = 0;

    void handle_field_events(Event &event, coords crds);
    void handle_interface_events(Event &event, Vector2f crds);
    void handle_keyboard_event(Event &event);
    void display_score(Vector2f mouse);
    void init_ng(uint8_t preset);


public:
    StandAloneApp(std::string font_path, std::string texture_path, std::string buttons_path);
    void init(uint16_t field_width, uint16_t field_hight, uint32_t mines_total);
    void main_loop();
};


StandAloneApp::StandAloneApp(std::string font_path, std::string texture_path, std::string buttons_path) {
    graph.reset(new Graphic(font_path, texture_path, buttons_path));
}

void StandAloneApp::init(uint16_t field_width, uint16_t field_hight, uint32_t mines_total) {
    graph->init_window(field_width, field_hight);
    field.reset(new Field(field_width, field_hight, mines_total));
}

void StandAloneApp::init_ng(uint8_t preset) {
    field.reset(new Field(preset));
    graph->init_window(field->field_width, field->field_hight);
    this->score = 0;
}


void StandAloneApp::handle_field_events(Event &event, coords crds) {
    if (field->state != field_state::INGAME && field->state != field_state::NEWGAME)
        return;
    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Right) {
        field->set_flag(crds);
    }
    if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Left) {
        field->open_cell(crds, score);
    }
}

void StandAloneApp::handle_interface_events(Event &event, Vector2f crds) {
    if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Left) {
        if (graph->buttons_spritesE.getGlobalBounds().contains(crds)) {
            init_ng(1);
        }
        if (graph->buttons_spritesN.getGlobalBounds().contains(crds)) {
            init_ng(2);
        }
        if (graph->buttons_spritesH.getGlobalBounds().contains(crds)) {
            init_ng(3);
        }
    }
}

void StandAloneApp::handle_keyboard_event(Event &event) {
    if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) {
        field->set_pause();
    }
}

void StandAloneApp::display_score(Vector2f mouse) {
    graph->draw_interface(
            (field->mines_total - field->flags_total),
            (field->ingame_time_total + field->ingame_time),
            field->get_state_text(),
            mouse
    );
    graph->draw_score("Sapper", score, 0, true, field->state != field_state::DEFEAT);
}

void StandAloneApp::main_loop() {
    bool is_inside_field = false;
    while (graph->window.isOpen()) {
		// Vector2i mouse_pos = Mouse::getPosition(app);
        Vector2f scaled_mouse_pos = graph->window.mapPixelToCoords(Mouse::getPosition(graph->window));
        coords crds(scaled_mouse_pos.x / graph->cell_size, (scaled_mouse_pos.y - graph->interface_shift) / graph->cell_size);

        Event event;
        while (graph->window.pollEvent(event)) {
            if (event.type == Event::Closed)
                graph->window.close();
            if (scaled_mouse_pos.y > graph->interface_shift
            && scaled_mouse_pos.y < graph->interface_shift + graph->field_h) {
                handle_field_events(event, crds);
                is_inside_field = true;
            } else {
                handle_interface_events(event, scaled_mouse_pos);
                is_inside_field = false;
            }
            handle_keyboard_event(event);
        }
		graph->window.clear(Color::White);
		for (int x = 0; x < field->field_width; x++) {
			for (int y = 0; y < field->field_hight; y++) {
                cell_condition sprite = field->get_sprite(coords(x, y),
                    is_inside_field && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left), crds);
                graph->draw_cell(x, y, int(sprite));
			}
        }
        field->update_time();
        display_score(scaled_mouse_pos);
		graph->window.display();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void load_preset(int preset, uint16_t &field_width, uint16_t &field_hight, uint32_t &total_mines) {
    switch (preset) {
        case 1: {
            field_width = 9;
            field_hight = 9;
            total_mines = 10;
            break;
        }
        case 2: {
            field_width = 16;
            field_hight = 16;
            total_mines = 40;
            break;
        }
        case 3:
            field_width = 30;
            field_hight = 16;
            total_mines = 99;
            break;
    }
}

void parse_args(int argc, char *argv[], uint16_t &field_width, uint16_t &field_hight, uint32_t &total_mines) {
    switch (argc) {
        case 2: {
            int preset;
            if ((preset = atoi(argv[1])) < 1 || preset > 3) {
                std::cerr << "wrong cmd argument" << std::endl;
                exit(1);
            } else {
                load_preset(preset, field_width, field_hight, total_mines);
                return;
            }
        }
        case 4: {
            if ((field_width = atoi(argv[1])) <= 0
                    || (field_hight = atoi(argv[2])) <= 0
                    || (total_mines = atoi(argv[3])) <= 0
                    || field_hight * field_width - total_mines < 10) {
                std::cerr << "wrong cmd argument" << std::endl;
                exit(1);
            }
            return;
        }
        // std::cerr << "wrong cmd argument\n";
        default: exit(1);
    }
}

int main(int argc, char *argv[]) {
    uint16_t field_width, field_hight;
    uint32_t mines_total;
    parse_args(argc, argv, field_width, field_hight, mines_total);

    StandAloneApp app("resources/arial.ttf", "resources/heb_tiles.png", "resources/buttons.png");

    app.init(field_width, field_hight, mines_total);

    app.main_loop();

    return 0;
}
