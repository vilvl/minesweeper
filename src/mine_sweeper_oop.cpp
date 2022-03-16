#include "main.h"

using namespace sf;

const int FIELD_HIGHT = 20;
const int FIELD_WIDTH = 20;
const int mine_probability = 5;

// cell_size pixels
const int cell_size = 32;
//field sizes
const int RES_HIGHT = cell_size * FIELD_HIGHT;
const int RES_WIDTH = cell_size * FIELD_WIDTH;

int main() {
    // std::unique_ptr<Field> field = nullptr;
    RenderWindow app(VideoMode(RES_HIGHT, RES_WIDTH), "MineSweeper");
    Texture cell_textures;
    cell_textures.loadFromFile("resources/tiles.jpg");
    Sprite cell_sprites(cell_textures);

    int gameover = 0;
    std::unique_ptr<Field> field(new Field(FIELD_HIGHT, FIELD_WIDTH, mine_probability, coords(0,0)));

    while (app.isOpen() && !gameover) {

		Vector2i pos = Mouse::getPosition(app);
		int x = pos.x / cell_size;
		int y = pos.y / cell_size;

        coords crds(x, y);

		Event event;
		while (app.pollEvent(event)) {
			if (event.type == Event::Closed)
				app.close();
            if (event.type == Event::MouseButtonPressed) {
                if (event.mouseButton.button == Mouse::Left) {
                    // if (!field)
                    //     field = std::unique_ptr<Field>(new Field(FIELD_HIGHT, FIELD_WIDTH, mine_probability, crds));
                    gameover = field->open_cell(crds);
                    if (!gameover)
                        gameover = field->check_win_condition();
                } else if (event.mouseButton.button == Mouse::Right) {
                    field->set_flag(crds);
                }
            }
		}
		app.clear(Color::White);
		for (int x = 0; x < FIELD_WIDTH; x++) {
			for (int y = 0; y < FIELD_HIGHT; y++) {
				cell_sprites.setTextureRect(IntRect(field->get_sprite(coords(x, y)) * cell_size, 0, cell_size, cell_size));
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