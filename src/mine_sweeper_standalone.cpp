#include "include/base.hpp"
#include "include/field.hpp"
#include "include/graphic.hpp"

#include <memory>


using namespace sf;


class StandAloneApp {
public:
    std::unique_ptr<Field> field;
    std::unique_ptr<Graphic> graph;

    void main_loop();
    void handle_field_events(Event &event, coords crds);
    void handle_interface_events(Event &event, Vector2f crds);
    void handle_keyboard_event(Event &event);
    void display_score();
};


void StandAloneApp::handle_field_events(Event &event, coords crds) {
    if (field->state != field_state::INGAME && field->state != field_state::NEWGAME)
        return;
    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left)
        graph->buttons_state.l = true;
    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Right) {
        graph->buttons_state.r = true;
        field->set_flag(crds);
    }
    if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Left) {
        graph->buttons_state.l = false;
        field->open_cell(crds);
    }
    if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Right) {
        graph->buttons_state.r = false;
    }
}

void StandAloneApp::handle_interface_events(Event &event, Vector2f crds) {
    if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Left) {
        uint16_t field_width = field->field_width;
        uint16_t field_hight = field->field_hight;
        uint32_t mines_total = field->mines_total;
        field.reset(new Field(field_width, field_hight, mines_total));
    }
}

void StandAloneApp::handle_keyboard_event(Event &event) {
    if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) {
        field->set_pause();
    }
}

void StandAloneApp::display_score() {
    std::string state_text;
    switch(field->state) {
        case field_state::WIN:       state_text = "GRAZ";         break;
        case field_state::DEFEAT:    state_text = "YOU LOST";     break;
        case field_state::PAUSE:     state_text = "PAUSE";        break;
        case field_state::NEWGAME:   state_text = "CLICK ON THE FIELD TO START";  break;
        case field_state::INGAME:    state_text = "";             break;
        default:        state_text = "WTF ITS DISPLAYED???";     break;
    }
    graph->draw_interface(
            std::to_string(field->mines_total - field->flags_total),
            std::to_string(field->ingame_time_total + field->ingame_time),
            state_text
    );
}

void StandAloneApp::main_loop() {
    while (graph->window.isOpen()) {
		// Vector2i mouse_pos = Mouse::getPosition(app);
        Vector2f scaled_mouse_pos = graph->window.mapPixelToCoords(Mouse::getPosition(graph->window));
        coords crds(scaled_mouse_pos.x / graph->cell_size, (scaled_mouse_pos.y - graph->interface_shift) / graph->cell_size);

        Event event;
        while (graph->window.pollEvent(event)) {
            if (event.type == Event::Closed)
                graph->window.close();
            if (scaled_mouse_pos.y >= graph->interface_shift) {
                handle_field_events(event, crds);
            } else {
                handle_interface_events(event, scaled_mouse_pos);
            }
            handle_keyboard_event(event);
        }
		graph->window.clear(Color::White);
		for (int x = 0; x < field->field_width; x++) {
			for (int y = 0; y < field->field_hight; y++) {
                graph->draw_cell(x, y, int(field->get_sprite(coords(x, y), graph->buttons_state.l, crds)));
			}
        }
        field->upate_time();
        display_score();
		graph->window.display();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void load_preset(int preset, u_short &field_width, u_short &field_hight, ushort &total_mines) {
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

void parse_args(int argc, char *argv[], u_short &field_width, u_short &field_hight, ushort &total_mines) {
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
    u_short field_width, field_hight, mines_total;
    parse_args(argc, argv, field_width, field_hight, mines_total);

    StandAloneApp app;
    app.graph.reset(new Graphic("resources/arial.ttf", "resources/heb_tiles.jpg"));
    // load textures

    app.graph->init_window(field_width, field_hight);
    app.field.reset(new Field(field_width, field_hight, mines_total));

    app.main_loop();

    return 0;
}

