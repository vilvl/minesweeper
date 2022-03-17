#include "include/field.hpp"

Field::Field(int field_width, int field_hight, int mines_total):
            field_width(field_width),
            field_hight(field_hight),
            mines_total(mines_total) {
    this->cells.reset(new std::vector<std::vector<FieldCell>>(
                      field_hight, std::vector<FieldCell>(field_width)));
    this->cells_total = field_hight * field_width;
}

Field::~Field() {}

FieldCell &Field::get_cell(coords crds) {
    return cells->at(crds.y)[crds.x];
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
            get_cell(coords(j, i)).set_mine(mine);
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
            get_cell(coords(j, i)).neighbors++;
}

void Field::count_neighbors() {
    // full field iterator
    for (int i = 0; i < field_hight; ++i)
        for (int j = 0; j < field_width; ++j)
            if (get_cell(coords(j, i)).is_mine)
                inc_neighbors(coords(j, i));
}

void Field::open_field() {
    // full field iterator
    for (int i = 0; i < field_hight; ++i)
        for (int j = 0; j < field_width; ++j) {
            FieldCell &cell = get_cell(coords(j, i));
            if (!cell.is_mine)
                cell.open_cell();
        }
}

void Field::open_cell_recursive(coords crds, bool first_iter) {
    int res = get_cell(crds).open_cell();
    if (res == -3) {
        this->state = DEFEAT;
        return;
    }
    if (res == -1 && first_iter)
        open_closed_neighbors(crds);
    if (res >= 0) {
        this->cells_opened++;
        if (res == 0)
            open_closed_neighbors(crds);
    }
}

void Field::open_cell(coords crds) {
    open_cell_recursive(crds, true);
    check_win_condition();
}

void Field::open_closed_neighbors(coords crds) {
    // neighbor iterator
    if (get_cell(crds).state != OPENED || get_cell(crds).neighbors != count_flaged_neighbors(crds))
        return;
    for (int i = std::max(crds.y - 1, 0); i < std::min(crds.y + 2, field_hight); i++)
        for (int j = std::max(crds.x - 1, 0); j < std::min(crds.x + 2, field_width); j++)
            open_cell_recursive(coords(j, i));
}

int Field::count_flaged_neighbors(coords crds) {
    // neighbor iterator
    int counter = 0;
    for (int i = std::max(crds.y - 1, 0); i < std::min(crds.y + 2, field_hight); i++)
        for (int j = std::max(crds.x - 1, 0); j < std::min(crds.x + 2, field_width); j++)
            if (get_cell(coords(j, i)).state == FLAGGED)
                counter++;
    return counter;
}

int Field::count_closed_neighbors(coords crds) {
    // neighbor iterator
    int counter = 0;
    for (int i = std::max(crds.y - 1, 0); i < std::min(crds.y + 2, field_hight); i++)
        for (int j = std::max(crds.x - 1, 0); j < std::min(crds.x + 2, field_width); j++)
            if (get_cell(coords(j, i)).state != OPENED)
                counter++;
    return counter;
}

void Field::flag_closed_neighbors(coords crds) {
    if (count_closed_neighbors(crds) != get_cell(crds).neighbors)
        return;
    for (int i = std::max(crds.y - 1, 0); i < std::min(crds.y + 2, field_hight); i++) {
        for (int j = std::max(crds.x - 1, 0); j < std::min(crds.x + 2, field_width); j++) {
            FieldCell &cell = get_cell(coords(j, i));
            if (cell.state == CLOSED)
                cell.state = FLAGGED;
        }
    }
}

void Field::check_win_condition() {
    if (state != DEFEAT)
        this->state = (mines_total + cells_opened == cells_total ? WIN : INGAME);
}

void Field::set_flag(coords crds) {
    FieldCell &cell = get_cell(crds);
    if (cell.state != OPENED) {
        cell.set_flag();
    } else {
        flag_closed_neighbors(crds);
    }
}

cell_condition Field::get_cell_condition(coords crds) {
    FieldCell &cell = get_cell(crds);
    switch (cell.state) {
        case CLOSED: return UNDEFINED;
        case FLAGGED: return FLAG;
        case OPENED:
            // if (cell.is_mine)
            //     return MINE;
            // else
            return cell_condition(cell.neighbors);
    }
}

cell_condition Field::get_true_condition(coords crds) {
    if (state != WIN && state != DEFEAT)
        return get_cell_condition(crds);
    FieldCell &cell = get_cell(crds);
    if (cell.state == FLAGGED && !cell.is_mine)
        return WRONG_FLAG;
    if (cell.state == OPENED && cell.is_mine)
        return EXPLODED;
    if (cell.state == CLOSED && cell.is_mine)
        return MINE;
    return get_cell_condition(crds);
}

bool Field::is_neighbors(coords pos1, coords pos2) {
    return (abs(pos1.x - pos2.x) <= 1 && abs(pos1.y - pos2.y) <= 1);
}