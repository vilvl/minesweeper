#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>

struct m_buttons_state {
    bool l = false;
    bool r = false;
    m_buttons_state(): l(false), r(false) {};
};

class Graphic {
public:
    sf::RenderWindow window;
    sf::Font font;
    sf::Texture cell_textures;
    sf::Sprite cell_sprites;
    sf::Text time_text;
    sf::Text mine_text;
    sf::Text state_text;
    sf::Text score_text;
    sf::Text my_score_text;

    int framerate = 30;

    int little_font_size = 18;
    int big_font_size = 26;
    int score_font_size = 10;

    int cell_size = 32;
    int interface_shift = 64;

    int field_w;
    int field_h;

    struct m_buttons_state buttons_state;

    void init_window(int field_w, int field_h);
    void set_sprites(std::string sprite_path);
    void set_font(std::string font_path);
    void draw_cell(int x, int y, int sprite);
    Graphic(std::string font_path, std::string sprites_path);
    void draw_interface(std::string mines, std::string time, std::string state);
    void draw_score(std::string &name, int16_t score, uint pos, bool its_me);
};
