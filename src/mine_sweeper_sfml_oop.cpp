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
    std::unique_ptr<Field> field;
    RenderWindow window;
    Font font;
    Texture cell_textures;
    Sprite cell_sprites;
    Text time_text;
    Text mine_text;
    Text state_text;

    // in pixels, not points!
    int little_font_size = 18;
    int big_font_size = 26;
    int cell_size = 32;
    // in cells
    int interface_shift = 64;
    int framerate = 30;
    struct m_buttons_state buttons_state;

    cell_condition get_sprite(coords current_cell_pos, coords mouse_pos);
    cell_condition get_end_sprite(coords current_cell_pos);
    void handle_field_events(Event &event, coords crds);
    void handle_interface_events(Event &event, Vector2f crds);
    void handle_keyboard_event(Event &event);
    void upate_time();
    void display_score();
    void main_loop();
    void init_window(int field_width, int field_hight);
    void set_sprites(std::string sprite_path);
    void set_font(std::string font_path);

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

void StandAloneApp::upate_time() {
    if (field->state == INGAME)
        field->ingame_time = std::chrono::duration_cast<std::chrono::seconds>
                                (std::chrono::steady_clock::now() - field->time_start).count();
}

void StandAloneApp::display_score() {
    mine_text.setString(std::to_string(field->mines_total - field->flags_total));
    mine_text.setPosition(Vector2f(cell_size / 2., (interface_shift - little_font_size) / 2.));
    window.draw(mine_text);

    time_text.setString(std::to_string(field->ingame_time_total + field->ingame_time));
    time_text.setPosition(Vector2f((field->field_width - 1.5) * cell_size, (interface_shift - little_font_size) / 2.));
    window.draw(time_text);

    if (field->state != INGAME) {
        int font_size = 26;
        switch(field->state) {
            case WIN:       state_text.setString("GRAZ");         break;
            case DEFEAT:    state_text.setString("YOU LOST");     break;
            case PAUSE:     state_text.setString("PAUSE");        break;
            case NEWGAME:   state_text.setString("CLICK ON THE FIELD TO START");  break;
            default:        state_text.setString("WHY IS THIS DISPLAYED???");     break;
        }
        state_text.setPosition(Vector2f((field->field_width / 2.) * cell_size - state_text.getString().getSize() * font_size / 2.5,
                                (interface_shift - font_size) / 2.));
        window.draw(state_text);
    }
}

void StandAloneApp::main_loop() {
    while (window.isOpen()) {
		// Vector2i mouse_pos = Mouse::getPosition(app);
        Vector2f scaled_mouse_pos = window.mapPixelToCoords(Mouse::getPosition(window));
        coords crds(scaled_mouse_pos.x / cell_size, (scaled_mouse_pos.y - interface_shift)/ cell_size);

        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
            if (scaled_mouse_pos.y >= interface_shift) {
                handle_field_events(event, crds);
            } else {
                handle_interface_events(event, scaled_mouse_pos);
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
                    sprite = get_sprite(coords(x, y), crds);
				cell_sprites.setTextureRect(IntRect(sprite * cell_size, 0, cell_size, cell_size));
				cell_sprites.setPosition(x * cell_size, interface_shift + y * cell_size);
                window.draw(cell_sprites);
			}
        }
        upate_time();
        display_score();
		window.display();
	}
}

void StandAloneApp::init_window(int field_width, int field_hight) {
    window.create(VideoMode(cell_size * field_width, interface_shift + field_hight * cell_size), "MineSweeper");
    window.setFramerateLimit(framerate);
}

void StandAloneApp::set_sprites(std::string sprite_path) {
    if (!cell_textures.loadFromFile(sprite_path)) {
        std::cerr << "sprites was not found" << std::endl;
        exit(2);
    }
    cell_textures.setSmooth(true);
    cell_sprites.setTexture(cell_textures);
}

void StandAloneApp::set_font(std::string font_path) {
    if (!font.loadFromFile(font_path)) {
        std::cerr << "sprites was not found" << std::endl;
        exit(2);
    }
    time_text.setFont(font);
    mine_text.setFont(font);
    state_text.setFont(font);

    time_text.setFillColor(Color::Red);
    mine_text.setFillColor(Color::Red);
    state_text.setFillColor(Color::Red);

    time_text.setStyle(sf::Text::Bold);
    mine_text.setStyle(sf::Text::Bold);
    state_text.setStyle(sf::Text::Bold);

    time_text.setCharacterSize(little_font_size);
    mine_text.setCharacterSize(little_font_size);
    state_text.setCharacterSize(big_font_size);
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

int main(int argc, char *argv[]) {
    u_short field_width, field_hight, mines_total;
    parse_args(argc, argv, field_width, field_hight, mines_total);

    StandAloneApp app;
    // load textures
    app.set_sprites("resources/heb_tiles.jpg");
    app.set_font("resources/arial.ttf");

    app.init_window(field_width, field_hight);
    app.field.reset(new Field(field_width, field_hight, mines_total));

    app.main_loop();

    return 0;
}
