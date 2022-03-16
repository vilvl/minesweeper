#include "field.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <utility>

#include <SFML/Graphics.hpp>

using namespace sf;

const int FIELD_HIGHT = 10;
const int FIELD_WIDTH = 10;
const int total_mines = 10;

// cell_size pixels
int cell_size = 32;

cell_condition get_sprite(cell_condition cond, coords current_cell_pos, coords mouse_pos, bool button_is_pressed) {
    if (cond == UNDEFINED && button_is_pressed && current_cell_pos == mouse_pos)
        return E0;
    return cond;
}

int main() {
    // std::unique_ptr<Field> field = nullptr;
    RenderWindow app(VideoMode(cell_size * FIELD_HIGHT, cell_size * FIELD_WIDTH), "MineSweeper");
    Texture cell_textures;
    cell_textures.loadFromFile("resources/tiles.jpg");
    Sprite cell_sprites(cell_textures);

    int gameover = 0;
    std::unique_ptr<Field> field(new Field(FIELD_HIGHT, FIELD_WIDTH, total_mines));

    bool l_button_is_pressed = false;
    bool r_button_is_pressed = false;

    while (app.isOpen() && !gameover) {

		Vector2i mouse_pos = Mouse::getPosition(app);
        Vector2f scaled_mouse_pos = app.mapPixelToCoords(mouse_pos);

        coords crds(scaled_mouse_pos.x / cell_size, scaled_mouse_pos.y / cell_size);
		Event event;
		while (app.pollEvent(event)) {
			if (event.type == Event::Closed)
				app.close();
            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left)
                l_button_is_pressed = true;
            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Right)
                r_button_is_pressed = true;
            if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Left) {
                l_button_is_pressed = false;
                if (r_button_is_pressed) {
                    gameover = field->open_closed_neighbors(crds);
                } else {
                    if (field->cells_opened == 0)
                        field->init(crds);
                    gameover = field->open_cell(crds);
                }
                if (!gameover)
                    gameover = field->check_win_condition();
            }
            if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Right) {
                r_button_is_pressed = false;
                field->set_flag(crds);
            }
		}

		app.clear(Color::White);
		for (int x = 0; x < FIELD_WIDTH; x++) {
			for (int y = 0; y < FIELD_HIGHT; y++) {
                cell_condition sprite = get_sprite(field->get_cell_condition(coords(x, y)), coords(x, y), crds, l_button_is_pressed);
				cell_sprites.setTextureRect(IntRect(sprite * cell_size, 0, cell_size, cell_size));
				cell_sprites.setPosition(x * cell_size, y * cell_size);
                app.draw(cell_sprites);
			}
        }
		app.display();
	}
    field.reset();
    if (app.isOpen())
        app.close();
    return 0;
}
