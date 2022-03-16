#include "field.hpp"


FieldCell *Field::get_cell(coords crds) {
    return &(cells[crds.y][crds.x]);
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

Field::Field(int field_hight, int field_width, int mines_total):
            field_width(field_width),
            field_hight(field_hight),
            mines_total(mines_total) {
    this->cells = get_cell_matrix(field_hight, field_width);
    this->cells_total = field_hight * field_width;
}

Field::~Field() {
    free_cell_matrix(cells, field_hight);
}

void Field::generate_field(coords start_crds) {
    int mine_probability = 100 * mines_total / cells_total;
    this->mines_total = 0;
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

void Field::init(coords start_crds) {
    generate_field(start_crds);
    count_neighbors();
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
    if (get_cell(crds)->state != OPENED || get_cell(crds)->neighbors != count_flaged_neighbors(crds))
        return 0;
    int ret = 0;
    for (int i = std::max(crds.y - 1, 0); i < std::min(crds.y + 2, field_hight); i++)
        for (int j = std::max(crds.x - 1, 0); j < std::min(crds.x + 2, field_width); j++)
            if (open_cell(coords(j, i)) == -1)
                ret = -1;
    return ret;
}

int Field::count_flaged_neighbors(coords crds) {
    // neighbor iterator
    int counter = 0;
    for (int i = std::max(crds.y - 1, 0); i < std::min(crds.y + 2, field_hight); i++)
        for (int j = std::max(crds.x - 1, 0); j < std::min(crds.x + 2, field_width); j++)
            if (get_cell(coords(j, i))->state == FLAGGED)
                counter++;
    return counter;
}

int Field::count_closed_neighbors(coords crds) {
    // neighbor iterator
    int counter = 0;
    for (int i = std::max(crds.y - 1, 0); i < std::min(crds.y + 2, field_hight); i++)
        for (int j = std::max(crds.x - 1, 0); j < std::min(crds.x + 2, field_width); j++)
            if (get_cell(coords(j, i))->state != OPENED)
                counter++;
    return counter;
}

void Field::flag_closed_neighbors(coords crds) {
    if (count_closed_neighbors(crds) != get_cell(crds)->neighbors)
        return;
    for (int i = std::max(crds.y - 1, 0); i < std::min(crds.y + 2, field_hight); i++) {
        for (int j = std::max(crds.x - 1, 0); j < std::min(crds.x + 2, field_width); j++) {
            auto cell = get_cell(coords(j, i));
            if (cell->state == CLOSED)
                cell->state = FLAGGED;
        }
    }
}

bool Field::check_win_condition() {
    return (mines_total + cells_opened == cells_total);
}

void Field::set_flag(coords crds) {
    auto cell = get_cell(crds);
    if (cell->state != OPENED) {
        cell->set_flag();
    } else {
        flag_closed_neighbors(crds);
    }
}

cell_condition Field::get_cell_condition(coords crds) {
    auto cell = get_cell(crds);
    switch (cell->state) {
        case CLOSED: return UNDEFINED;
        case FLAGGED: return FLAG;
        case OPENED:
            if (cell->is_mine)
                return MINE;
            else
                return cell_condition(cell->neighbors);
    }
}
