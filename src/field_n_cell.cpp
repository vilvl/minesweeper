#include "main.h"

class FieldCell {
private:
public:
    bool is_mine;
    bool is_open = false;
    bool is_flag = false;
    int neighbors = 0;
    cell_condition sprite = UNDEFINED;

public:
    void set_mine(bool state) {
        this->is_mine = state;
    };
    void set_flag() {
        if (!is_open) {
            this->is_flag = !this->is_flag;
            this->sprite = (this->is_flag ? FLAG : UNDEFINED);
        }
    };
    int open_cell() {
        if (is_flag || is_open)
            return -1;
        this->is_open = true;
        if (is_mine) {
            sprite = MINE;
            return -2;
        } else {
            sprite = cell_condition(neighbors);
            return neighbors;
        }
    };
};

FieldCell *Field::get_cell(coords crds) {
    return &(cells[crds.y][crds.x]);
}

cell_condition Field::get_sprite(coords crds) {
    return get_cell(crds)->sprite;
}

void free_cell_matrix(FieldCell **matr, int rws) {
    for (int j = 0; j < rws; ++j) {
        delete(matr[j]);
    }
    delete matr;
}

FieldCell **get_cell_matrix(int rws, int cls) {
    FieldCell **matr = new FieldCell*[rws];
    for (int j = 0; j < rws; ++j) {
        matr[j] = new FieldCell[cls];
    }
    return matr;
}

void Field::generate_field(int mine_probability, coords start_crds) {
    srand(time(NULL));
    // full field iterator
    for (int i = 0; i < field_hight; ++i) {
        for (int j = 0; j < field_width; ++j) {
            bool mine = false;
            if (i != start_crds.y || j != start_crds.x)
                mine = (rand() % 100 < mine_probability);
            get_cell(coords(j, i))->set_mine(mine);
            this->mines_total += mine;
        }
    }
}

Field::Field(int field_hight, int field_width, int mine_probability, coords start_crds):
            field_width(field_width),
            field_hight(field_hight) {
    this->cells = get_cell_matrix(field_hight, field_width);
    this->cells_total = field_hight * field_width;
    generate_field(mine_probability, start_crds);
    count_neighbors();
}

Field::~Field() {
    free_cell_matrix(cells, field_hight);
}

void Field::open_field() {
    // full field iterator
    for (int i = 0; i < field_hight; ++i)
        for (int j = 0; j < field_width; ++j)
            get_cell(coords(j, i))->open_cell();
}

int Field::open_cell(coords crds) {
    int res = get_cell(crds)->open_cell();
    if (res == -2)
        return -1;
    if (res >= 0) {
        this->cells_opened++;
        if (res == 0)
            return this->open_closed_neighbors(crds);
    }
    return 0;
}

int Field::open_closed_neighbors(coords crds) {
    // neighbor iterator
    for (int i = std::max(crds.y - 1, 0); i < std::min(crds.y + 2, field_hight); i++)
        for (int j = std::max(crds.x - 1, 0); j < std::min(crds.x + 2, field_width); j++)
            if (open_cell(coords(j, i)) == -1)
                return -1;
    return 0;
}

void Field::inc_neighbors(coords crds) {
    // neighbor iterator
    for (int i = std::max(crds.y - 1, 0); i < std::min(crds.y + 2, field_hight); i++)
        for (int j = std::max(crds.x - 1, 0); j < std::min(crds.x + 2, field_width); j++)
            get_cell(coords(j, i))->neighbors++;
}

void Field::count_neighbors() {
    // full field iterator
    for (int i = 0; i < field_hight; ++i)
        for (int j = 0; j < field_width; ++j)
            if (get_cell(coords(j, i))->is_mine)
                inc_neighbors(coords(j, i));
}

int Field::count_closed_neighbors(coords crds) {
    // neighbor iterator
    int counter = 0;
    for (int i = std::max(crds.y - 1, 0); i < std::min(crds.y + 2, field_hight); i++)
        for (int j = std::max(crds.x - 1, 0); j < std::min(crds.x + 2, field_width); j++)
            if (!get_cell(coords(j, i))->is_open)
                counter++;
    return counter;
}

int Field::count_flaged_neighbors(coords crds) {
    // neighbor iterator
    int counter = 0;
    for (int i = std::max(crds.y - 1, 0); i < std::min(crds.y + 2, field_hight); i++)
        for (int j = std::max(crds.x - 1, 0); j < std::min(crds.x + 2, field_width); j++)
            if (!get_cell(coords(j, i))->is_flag)
                counter++;
    return counter;
}

bool Field::check_win_condition() {
    return (mines_total + cells_opened == cells_total);
}

void Field::set_flag(coords crds) {
    get_cell(crds)->set_flag();
}