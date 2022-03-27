#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>

// struct m_buttons_state {
//     bool l = false;
//     bool r = false;
//     m_buttons_state(): l(false), r(false) {};
// };

class Graphic {
public:
    sf::RenderWindow window;
    sf::Font font;
    sf::Texture cell_textures;
    sf::Sprite cell_sprites;
    sf::Texture buttons_textures;
    sf::Sprite buttons_spritesE;
    sf::Sprite buttons_spritesN;
    sf::Sprite buttons_spritesH;
    sf::Text time_text;
    sf::Text mine_text;
    sf::Text state_text;
    sf::Text score_text;

    int framerate = 30;

    int little_font_size = 20;
    int big_font_size = 32;
    int score_font_size = 20;

    int cell_size = 32;
    int interface_shift = 64;
    int score_shift = 4 * cell_size;

    int field_w;
    int field_h;

    // struct m_buttons_state buttons_state;

    void init_window(int field_w, int field_h);
    void set_sprites(std::string sprite_path, std::string buttons_path);
    void set_font(std::string font_path);
    void draw_cell(int x, int y, int sprite);
    Graphic(std::string font_path, std::string sprites_path, std::string buttons_path);
    void draw_interface(uint32_t mines, uint32_t time, std::string state, sf::Vector2f mouse);
    void draw_score(const std::string name, const int16_t score, const uint pos, const bool its_me, const bool active);
};
