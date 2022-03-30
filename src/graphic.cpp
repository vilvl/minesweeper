#include "include/graphic.hpp"

using namespace sf;

Graphic::Graphic(std::string font_path, std::string sprite_path, std::string buttons_path) {
    set_font(font_path);
    set_sprites(sprite_path, buttons_path);
}

void Graphic::init_window(int field_width, int field_hight) {
    if (window.isOpen() && field_w == field_width * cell_size && field_h == field_hight * cell_size)
        return;

    this->field_w = field_width * cell_size;
    this->field_h = field_hight * cell_size;

    if (window.isOpen()) {
        Vector2i pos(window.getPosition());
        window.create(VideoMode(field_w, interface_shift + field_h + score_shift), "MineSweeper");
        window.setFramerateLimit(framerate);
        window.setPosition(pos);
    } else {
        window.create(VideoMode(field_w, interface_shift + field_h + score_shift), "MineSweeper");
        window.setFramerateLimit(framerate);
    }
}

void Graphic::set_sprites(std::string sprite_path, std::string buttons_path) {
    if (!cell_textures.loadFromFile(sprite_path) || !buttons_textures.loadFromFile(buttons_path)) {
        std::cerr << "sprites was not found" << std::endl;
        exit(2);
    }
    // cell_textures.setSmooth(true);
    cell_sprites.setTexture(cell_textures);
    buttons_spritesE.setTexture(buttons_textures);
    buttons_spritesN.setTexture(buttons_textures);
    buttons_spritesH.setTexture(buttons_textures);
}

void Graphic::set_font(std::string font_path) {
    if (!font.loadFromFile(font_path)) {
        std::cerr << "font was not found" << std::endl;
        exit(2);
    }
    time_text.setFont(font);
    mine_text.setFont(font);
    state_text.setFont(font);
    score_text.setFont(font);

    time_text.setFillColor(Color::Red);
    mine_text.setFillColor(Color::Red);
    state_text.setFillColor(Color::Red);
    // score_text.setFillColor(Color::Red);

    time_text.setStyle(sf::Text::Bold);
    mine_text.setStyle(sf::Text::Bold);
    state_text.setStyle(sf::Text::Bold);
    score_text.setStyle(sf::Text::Bold);

    time_text.setCharacterSize(little_font_size);
    mine_text.setCharacterSize(little_font_size);
    state_text.setCharacterSize(big_font_size);
    score_text.setCharacterSize(score_font_size);
}

void Graphic::draw_cell(int x, int y, int sprite) {
    cell_sprites.setTextureRect(IntRect(sprite * cell_size, 0, cell_size, cell_size));
    cell_sprites.setPosition(x * cell_size, interface_shift + y * cell_size);
    window.draw(cell_sprites);
}

void Graphic::draw_interface(int32_t mines, uint32_t time, std::string state, Vector2f mouse) {
    mine_text.setString(std::to_string(mines));
    mine_text.setPosition(Vector2f(cell_size / 2.,
                    (interface_shift - little_font_size) / 2.));
    window.draw(mine_text);

    time_text.setString(std::to_string(time));
    time_text.setPosition(Vector2f(field_w - 1.5 * cell_size,
                    (interface_shift - little_font_size) / 2.));
    window.draw(time_text);

    if (!state.empty()) {
        state_text.setString(state);
        sf::FloatRect textRect = state_text.getLocalBounds();
        state_text.setOrigin(textRect.left + textRect.width/2.0f,
               textRect.top  + textRect.height/2.0f);
        sf::Vector2f center = sf::Vector2f(window.getView().getCenter());
        center.y = interface_shift + field_h/2.;
        state_text.setPosition(center);
        window.draw(state_text);
    }

    buttons_spritesE.setTextureRect(IntRect(5 * cell_size, 0, cell_size * 5, cell_size));
    buttons_spritesE.setPosition(field_w - cell_size * 5.5,
                    interface_shift + field_h + 0.5 * cell_size);
    buttons_spritesN.setTextureRect(IntRect(15 * cell_size, 0, cell_size * 5, cell_size));
    buttons_spritesN.setPosition(field_w - cell_size * 5.5,
                    interface_shift + field_h + 1.5 * cell_size);
    buttons_spritesH.setTextureRect(IntRect(25 * cell_size, 0, cell_size * 5, cell_size));
    buttons_spritesH.setPosition(field_w - cell_size * 5.5,
                    interface_shift + field_h + 2.5 * cell_size);
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        if (buttons_spritesE.getGlobalBounds().contains(mouse))
            buttons_spritesE.setTextureRect(IntRect(0 * cell_size, 0, cell_size * 5, cell_size));
        if (buttons_spritesN.getGlobalBounds().contains(mouse))
            buttons_spritesN.setTextureRect(IntRect(10 * cell_size, 0, cell_size * 5, cell_size));
        if (buttons_spritesH.getGlobalBounds().contains(mouse))
            buttons_spritesH.setTextureRect(IntRect(20 * cell_size, 0, cell_size * 5, cell_size));
    }
    window.draw(buttons_spritesE);
    window.draw(buttons_spritesN);
    window.draw(buttons_spritesH);
}

void Graphic::draw_score(const std::string name, const int16_t score,
            const unsigned pos, const bool its_me, const bool active) {
    if (!active)
        score_text.setFillColor(sf::Color::Red);
    else if (its_me)
        score_text.setFillColor(sf::Color::Magenta);
    else
        score_text.setFillColor(sf::Color::Blue);

    score_text.setString(name + ": " + std::to_string(score));
    score_text.setPosition(Vector2f(cell_size / 2.,
            interface_shift + field_h + cell_size / 2. + cell_size * pos));
    window.draw(score_text);
}
