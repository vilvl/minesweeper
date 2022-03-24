#include "include/field.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <utility>

#include <SFML/Graphics.hpp>

using namespace sf;

// cell_size pixels
int cell_size = 32;
int interface_shift = 2;

struct m_buttons_state {
    bool l = false;
    bool r = false;
    m_buttons_state(bool l, bool r): l(l), r(r) {};
};

cell_condition get_end_sprite(std::unique_ptr<Field> &field, coords current_cell_pos) {
    return field->get_true_condition(current_cell_pos);
}

cell_condition get_sprite(std::unique_ptr<Field> &field, coords current_cell_pos, coords mouse_pos, bool button_is_pressed) {
    cell_condition cond = field->get_cell_condition(current_cell_pos);
    if (cond == UNDEFINED
    && button_is_pressed
    && current_cell_pos == mouse_pos)
        return E0;
    if (cond == UNDEFINED
    && button_is_pressed
    && field->get_cell_condition(mouse_pos) >= cell_condition(0)
    && field->get_cell_condition(mouse_pos) <= cell_condition(8)
    && field->is_neighbors(current_cell_pos, mouse_pos))
        return E0;
    return cond;
}

void handle_field_events(Event &event, std::unique_ptr<Field> &field,
                         coords crds, m_buttons_state &m_buttons) {
    if (field->state != INGAME && field->state != NEWGAME)
        return;
    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left)
        m_buttons.l = true;
    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Right) {
        m_buttons.r = true;
        field->set_flag(crds);
    }
    if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Left) {
        m_buttons.l = false;
        field->open_cell(crds);
    }
    if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Right) {
        m_buttons.r = false;
    }
    // if (event.type == Event::KeyPressed && event.key == Keyboard::Escape)
    //     field->state = PAUSE;
    if (field->state == WIN || field->state == DEFEAT) {
        field->ingame_time_total += field->ingame_time;
        field->ingame_time = 0;
    }
}

void handle_interface_events(Event &event, std::unique_ptr<Field> &field,
                             coords crds, m_buttons_state &m_buttons) {
    if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Left) {
        uint16_t field_width = field->field_width;
        uint16_t field_hight = field->field_hight;
        uint32_t mines_total = field->mines_total;
        field.reset(new Field(field_width, field_hight, mines_total));
    }
}

void handle_keyboard_event(Event &event, std::unique_ptr<Field> &field) {
    if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) {
        field->set_pause();
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
            field_hight = 16;
            field_width = 16;
            total_mines = 40;
            break;
        }
        case 3:
            field_hight = 16;
            field_width = 30;
            total_mines = 99;
            break;
    }
}

void parse_args(int argc, char *argv[], u_short &field_width, u_short &field_hight, ushort &total_mines) {
    switch (argc) {
        case 2: {
            int preset;
            if ((preset = atoi(argv[1])) < 0 || preset > 3) {
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

void upate_time(std::unique_ptr<Field> &field) {
    if (field->state == INGAME)
        field->ingame_time = std::chrono::duration_cast<std::chrono::seconds>
                                (std::chrono::steady_clock::now() - field->time_start).count();
}

void display_score(RenderWindow &app, std::unique_ptr<Field> &field, Font &font) {
    Text text_field;
    text_field.setFont(font);
    text_field.setFillColor(Color::Red);
    text_field.setStyle(sf::Text::Bold);

    int font_size = 18;
    std::string s = std::to_string(field->mines_total - field->flags_total);
    text_field.setCharacterSize(font_size); // in pixels, not points!
    text_field.setString(s);
    text_field.setPosition(Vector2f(cell_size / 2., (cell_size * interface_shift - font_size) / 2.));
    app.draw(text_field);
    s = std::to_string(field->ingame_time_total + field->ingame_time);
    text_field.setCharacterSize(font_size); // in pixels, not points!
    text_field.setString(s);
    text_field.setPosition(Vector2f((field->field_width - 1.5) * cell_size, (cell_size * interface_shift - font_size) / 2.));
    app.draw(text_field);

    if (field->state == WIN || field->state == DEFEAT || field->state == PAUSE) {
        int font_size = 26;
        s = (field->state == WIN ? "GRAZ!" : (field->state == DEFEAT ? "YOU LOST" : "PAUSE"));
        text_field.setCharacterSize(32); // in pixels, not points!
        text_field.setString(s);
        // text_field.
        text_field.setPosition(Vector2f((field->field_width / 2.) * cell_size - s.length() * font_size / 2.,
                                (cell_size * interface_shift - font_size) / 2.));
        app.draw(text_field);
    }
}

int main(int argc, char *argv[]) {
    u_short field_width, field_hight, mines_total;
    parse_args(argc, argv, field_width, field_hight, mines_total);

    RenderWindow app(VideoMode(cell_size * field_width,  (interface_shift + field_hight) * cell_size), "MineSweeper");
    app.setFramerateLimit(60);

    // load textures
    Texture cell_textures;
    if (!cell_textures.loadFromFile("resources/heb_tiles.jpg")) {
        exit(2);
    }
    Sprite cell_sprites(cell_textures);

    Font font;
    if (!font.loadFromFile("resources/arial.ttf")) {
        exit(2);
    }

    std::unique_ptr<Field> field(new Field(field_width, field_hight, mines_total));

    m_buttons_state m_buttons(false, false);

    while (app.isOpen()) {

		// Vector2i mouse_pos = Mouse::getPosition(app);
        Vector2f scaled_mouse_pos = app.mapPixelToCoords(Mouse::getPosition(app));
        coords crds(scaled_mouse_pos.x / cell_size, scaled_mouse_pos.y / cell_size);

        Event event;
        while (app.pollEvent(event)) {
            if (event.type == Event::Closed)
                app.close();
            if (crds.y >= interface_shift) {
                handle_field_events(event, field, coords(crds.x, crds.y - interface_shift), m_buttons);
            } else {
                handle_interface_events(event, field, crds, m_buttons);
            }
            handle_keyboard_event(event, field);
        }
		app.clear(Color::White);
		for (int x = 0; x < field->field_width; x++) {
			for (int y = 0; y < field->field_hight; y++) {
                cell_condition sprite;
                if (field->state == WIN || field->state == DEFEAT)
                    sprite = get_end_sprite(field, coords(x, y));
                else
                    sprite = get_sprite(field, coords(x, y), coords(crds.x, crds.y - interface_shift), m_buttons.l);
				cell_sprites.setTextureRect(IntRect(sprite * cell_size, 0, cell_size, cell_size));
				cell_sprites.setPosition(x * cell_size, (interface_shift + y) * cell_size);
                app.draw(cell_sprites);
			}
        }
        upate_time(field);
        display_score(app, field, font);
		app.display();
	}

    field.reset();
    if (app.isOpen())
        app.close();
    return 0;
}
