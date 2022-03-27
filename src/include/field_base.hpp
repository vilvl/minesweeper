#pragma once

#include <chrono>
#include <iostream>

enum class cell_condition {
    E0 = 0,
    E1,
    E2,
    E3,
    E4,
    E5,
    E6,
    E7,
    E8,
    MINE,
    UNDEFINED,
    FLAG,
    WRONG_FLAG,
    EXPLODED,
};

enum class field_state {
    NEWGAME,
    WIN,
    DEFEAT,
    INGAME,
    PAUSE,
    NOTINITED,
};

struct coords {
public:
    int16_t x;
    int16_t y;
    coords(int x, int y): x(x), y(y) {};
    bool operator==(coords& other) const {
        return (x == other.x && y == other.y);
    }
};

class FieldBase {
public:
    uint16_t field_width;
    uint16_t field_hight;
    uint32_t cells_total;
    uint32_t mines_total;
    uint32_t flags_total = 0;
    field_state state = field_state::NEWGAME;
    std::chrono::steady_clock::time_point time_start;
    uint32_t ingame_time = 0;
    uint32_t ingame_time_total = 0;

    FieldBase(uint8_t preset) {
        load_preset(preset);
        this->cells_total = field_hight * field_width;
    };

    FieldBase(uint16_t field_width, uint16_t field_hight, uint32_t mines_total):
            field_width(field_width),
            field_hight(field_hight),
            cells_total(field_hight * field_width),
            mines_total(mines_total) {}

    std::string get_state_text() {
        switch(state) {
            case field_state::WIN:       return "YOU WIN!";
            case field_state::DEFEAT:    return "YOU LOST";
            case field_state::PAUSE:     return "PAUSE";
            case field_state::NEWGAME:   return "CLICK TO START";
            case field_state::INGAME:    return "";
            default: return "WTF ITS DISPLAYED???";
        }
    }

    void update_time() {
        if (state == field_state::INGAME)
            this->ingame_time = std::chrono::duration_cast<std::chrono::seconds>
                    (std::chrono::steady_clock::now() - time_start).count();
    };

    void load_preset(int preset) {
        switch (preset) {
            case 2: {
                this->field_width = 16;
                this->field_hight = 16;
                this->mines_total = 40;
                break;
            }
            case 3:
                this->field_width = 30;
                this->field_hight = 16;
                this->mines_total = 99;
                break;
            default: {
                this->field_width = 9;
                this->field_hight = 9;
                this->mines_total = 10;
                break;
            }
        }
    }

    bool is_neighbors(coords pos1, coords pos2) {
        if (pos1.x < 0 || pos2.x < 0 || pos1.y < 0 || pos2.y < 0
        || pos1.x >= field_width || pos2.x >= field_width || pos1.y >= field_hight || pos2.y >= field_hight)
            return false;
        return (abs(pos1.x - pos2.x) <= 1 && abs(pos1.y - pos2.y) <= 1);
    }
};
