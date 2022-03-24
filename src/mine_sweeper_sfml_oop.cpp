#include "include/field.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <utility>

#include <SFML/Graphics.hpp>

using namespace sf;

struct m_buttons_state {
    bool l = false;
    bool r = false;
    m_buttons_state(): l(false), r(false) {};
};

class StandAloneApp {
public:
    RenderWindow window;
    std::unique_ptr<Field> field;
    Font font;
    Texture cell_textures;
    Sprite cell_sprites;

    int cell_size = 32;
    int interface_shift = 2;
    int framerate = 30;
    struct m_buttons_state buttons_state;

    cell_condition get_sprite(coords current_cell_pos, coords mouse_pos);
    cell_condition get_end_sprite(coords current_cell_pos);
    void handle_field_events(Event &event, coords crds);
    void handle_interface_events(Event &event, coords crds);
    void handle_keyboard_event(Event &event);
    void upate_time();
    void display_score();
    void main_loop();
    void init_window(int field_width, int field_hight);
    StandAloneApp() {};

};

cell_condition StandAloneApp::get_end_sprite(coords current_cell_pos) {
    return field->get_true_condition(current_cell_pos);
}

cell_condition StandAloneApp::get_sprite(coords current_cell_pos, coords mouse_pos) {
    cell_condition cond = field->get_cell_condition(current_cell_pos);
    if (cond == UNDEFINED
    && buttons_state.l
    && current_cell_pos == mouse_pos)
        return E0;
    if (cond == UNDEFINED
    && buttons_state.l
    && field->get_cell_condition(mouse_pos) >= cell_condition(0)
    && field->get_cell_condition(mouse_pos) <= cell_condition(8)
    && field->is_neighbors(current_cell_pos, mouse_pos))
        return E0;
    return cond;
}

void StandAloneApp::handle_field_events(Event &event, coords crds) {
    if (field->state != INGAME && field->state != NEWGAME)
        return;
    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left)
        buttons_state.l = true;
    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Right) {
        buttons_state.r = true;
        field->set_flag(crds);
    }
    if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Left) {
        buttons_state.l = false;
        field->open_cell(crds);
    }
    if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Right) {
        buttons_state.r = false;
    }
    // if (event.type == Event::KeyPressed && event.key == Keyboard::Escape)
    //     field->state = PAUSE;
    if (field->state == WIN || field->state == DEFEAT) {
        field->ingame_time_total += field->ingame_time;
        field->ingame_time = 0;
    }
}

void StandAloneApp::handle_interface_events(Event &event, coords crds) {
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

void StandAloneApp::upate_time() {
    if (field->state == INGAME)
        field->ingame_time = std::chrono::duration_cast<std::chrono::seconds>
                                (std::chrono::steady_clock::now() - field->time_start).count();
}

void StandAloneApp::display_score() {
    Text text_field;
    text_field.setFont(font);
    text_field.setFillColor(Color::Red);
    text_field.setStyle(sf::Text::Bold);

    int font_size = 18;
    std::string s = std::to_string(field->mines_total - field->flags_total);
    text_field.setCharacterSize(font_size); // in pixels, not points!
    text_field.setString(s);
    text_field.setPosition(Vector2f(cell_size / 2., (cell_size * interface_shift - font_size) / 2.));
    window.draw(text_field);
    s = std::to_string(field->ingame_time_total + field->ingame_time);
    text_field.setCharacterSize(font_size); // in pixels, not points!
    text_field.setString(s);
    text_field.setPosition(Vector2f((field->field_width - 1.5) * cell_size, (cell_size * interface_shift - font_size) / 2.));
    window.draw(text_field);

    if (field->state != INGAME) {
        int font_size = 26;
        switch(field->state) {
            case WIN:       s = "GRAZ";         break;
            case DEFEAT:    s = "YOU LOST";     break;
            case PAUSE:     s = "PAUSE";        break;
            case NEWGAME:   s = "CLICK ON THE FIELD TO START";  break;
            default:        s = "WHY IS THIS DISPLAYED???";     break;
        }
        text_field.setCharacterSize(32); // in pixels, not points!
        text_field.setString(s);
        text_field.setPosition(Vector2f((field->field_width / 2.) * cell_size - s.length() * font_size / 2.,
                                (cell_size * interface_shift - font_size) / 2.));
        window.draw(text_field);
    }
}


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

void StandAloneApp::main_loop() {
    while (window.isOpen()) {

		// Vector2i mouse_pos = Mouse::getPosition(app);
        Vector2f scaled_mouse_pos = window.mapPixelToCoords(Mouse::getPosition(window));
        coords crds(scaled_mouse_pos.x / cell_size, scaled_mouse_pos.y / cell_size);

        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
            if (crds.y >= interface_shift) {
                handle_field_events(event, coords(crds.x, crds.y - interface_shift));
            } else {
                handle_interface_events(event, crds);
            }
            handle_keyboard_event(event);
        }
		window.clear(Color::White);
		for (int x = 0; x < field->field_width; x++) {
			for (int y = 0; y < field->field_hight; y++) {
                cell_condition sprite;
                if (field->state == WIN || field->state == DEFEAT)
                    sprite = get_end_sprite(coords(x, y));
                else
                    sprite = get_sprite(coords(x, y), coords(crds.x, crds.y - interface_shift));
				cell_sprites.setTextureRect(IntRect(sprite * cell_size, 0, cell_size, cell_size));
				cell_sprites.setPosition(x * cell_size, (interface_shift + y) * cell_size);
                window.draw(cell_sprites);
			}
        }
        upate_time();
        display_score();
		window.display();
	}
}

void StandAloneApp::init_window(int field_width, int field_hight) {
    window.create(VideoMode(cell_size * field_width, (interface_shift + field_hight) * cell_size), "MineSweeper");
    window.setFramerateLimit(framerate);
}

int main(int argc, char *argv[]) {
    u_short field_width, field_hight, mines_total;
    parse_args(argc, argv, field_width, field_hight, mines_total);

    StandAloneApp app;
    app.init_window(field_width, field_hight);

    // load textures
    if (!app.cell_textures.loadFromFile("resources/heb_tiles.jpg")) {
        std::cerr << "sprites was not found" << std::endl;
        exit(2);
    }
    app.cell_sprites.setTexture(app.cell_textures);


    if (!app.font.loadFromFile("resources/arial.ttf")) {
        std::cerr << "sprites was not found" << std::endl;
        exit(2);
    }

    app.field.reset(new Field(field_width, field_hight, mines_total));

    app.main_loop();

    return 0;
}
