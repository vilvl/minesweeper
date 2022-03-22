#include "include/field.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <utility>

#include <SFML/Graphics.hpp>

using namespace sf;


class CliField {
private:
    std::unique_ptr<std::vector<std::vector<cell_condition>>> cells;

public:
    int field_width;
    int field_hight;
    int mines_total;
    int flags_total = 0;
    FieldState state = NEWGAME;
    u_int ingame_time_total = 0;

public:
    CliField(int field_width, int field_hight, int mines_total);
    ~CliField();
    void open_cell(coords crds);
    void set_flag(coords crds);
    cell_condition &get_cell(coords crds);
};

int main() {

    RenderWindow settings(VideoMode(200,  300), "Settings");

    Font font;
    if (!font.loadFromFile("resources/arial.ttf")) {
        exit(2);
    }

    

}