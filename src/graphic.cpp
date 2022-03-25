#include "include/graphic.hpp"

using namespace sf;

Graphic::Graphic(std::string font_path, std::string sprite_path) {
    set_font(font_path);
    set_sprites(sprite_path);
}

void Graphic::init_window(int field_width, int field_hight) {
    this->field_w = field_width * cell_size;
    this->field_h = field_hight * cell_size;

    window.create(VideoMode(field_w, interface_shift + field_h), "MineSweeper");
    window.setFramerateLimit(framerate);
}

void Graphic::set_sprites(std::string sprite_path) {
    if (!cell_textures.loadFromFile(sprite_path)) {
        std::cerr << "sprites was not found" << std::endl;
        exit(2);
    }
    cell_textures.setSmooth(true);
    cell_sprites.setTexture(cell_textures);
}

void Graphic::set_font(std::string font_path) {
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

void Graphic::draw_cell(int x, int y, int sprite) {
    cell_sprites.setTextureRect(IntRect(sprite * cell_size, 0, cell_size, cell_size));
    cell_sprites.setPosition(x * cell_size, interface_shift + y * cell_size);
    window.draw(cell_sprites);
}

void Graphic::draw_interface(std::string mines, std::string time, std::string state) {
    mine_text.setString(mines);
    mine_text.setPosition(Vector2f(cell_size / 2., (interface_shift - little_font_size) / 2.));
    window.draw(mine_text);

    time_text.setString(time);
    time_text.setPosition(Vector2f(field_w - 1.5 * cell_size, (interface_shift - little_font_size) / 2.));
    window.draw(time_text);

    if (!state.empty()) {
        state_text.setString(state);
        state_text.setPosition(Vector2f(field_w / 2 - state.length() * big_font_size / 2.5, (interface_shift - big_font_size) / 2.));
        window.draw(state_text);
    }
}
