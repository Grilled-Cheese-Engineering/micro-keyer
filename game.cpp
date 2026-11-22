#include "display.h"
#include <string>
#include <random>
#include <math.h>
#include <format>
#include <iostream>
#include <cstdlib>
#include <ctime>

int highScore;

bool dino[21][40] = { { false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, true, true, true, false },{ false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, true, true, true, true, true },{ false, false, false, false, false, false, false, false, false, false, true, true, false, true, true, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, true, true, false, true, true, true, true, true, true, true },{ false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, true, true, true, true, true },{ false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, true, true, true, true, true },{ false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, false, false, false, false, false },{ false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, true, true, true, false, false },{ true, false, false, false, false, false, false, false, false, true, true, true, true, true, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, true, true, true, true, true, false, false, false, false, false, false },{ true, false, false, false, false, false, false, true, true, true, true, true, true, true, false, false, false, false, false, false, true, false, false, false, false, false, false, true, true, true, true, true, true, true, false, false, false, false, false, false },{ true, true, false, false, false, false, true, true, true, true, true, true, true, true, true, true, false, false, false, false, true, true, false, false, false, false, true, true, true, true, true, true, true, true, true, true, false, false, false, false },{ true, true, true, false, false, true, true, true, true, true, true, true, true, true, false, true, false, false, false, false, true, true, true, false, false, true, true, true, true, true, true, true, true, true, false, true, false, false, false, false },{ true, true, true, true, true, true, true, true, true, true, true, true, true, true, false, false, false, false, false, false, true, true, true, true, true, true, true, true, true, true, true, true, true, true, false, false, false, false, false, false },{ true, true, true, true, true, true, true, true, true, true, true, true, true, true, false, false, false, false, false, false, true, true, true, true, true, true, true, true, true, true, true, true, true, true, false, false, false, false, false, false },{ false, true, true, true, true, true, true, true, true, true, true, true, true, true, false, false, false, false, false, false, false, true, true, true, true, true, true, true, true, true, true, true, true, true, false, false, false, false, false, false },{ false, false, true, true, true, true, true, true, true, true, true, true, true, false, false, false, false, false, false, false, false, false, true, true, true, true, true, true, true, true, true, true, true, false, false, false, false, false, false, false },{ false, false, false, true, true, true, true, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, true, true, true, true, false, false, false, false, false, false, false, false },{ false, false, false, false, true, true, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, true, true, false, false, false, false, false, false, false, false, false },{ false, false, false, false, false, true, true, false, false, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, false, false, true, true, false, false, false, false, false, false, false, false, false },{ false, false, false, false, false, false, true, true, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, true, true, false, false, false, false, false, false, false, false },{ false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false },{ false, false, false, false, false, false, false, false, false, false, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, false, false, false, false, false, false, false, false, false, false, false, false } };
bool cactus[24][15] = { { false, false, false, false, false, false, true, true, true, false, false, false, false, false, false },{ false, false, false, false, false, true, true, true, true, false, false, false, false, false, false },{ false, true, true, false, false, true, true, true, true, true, false, false, false, false, false },{ true, true, true, true, false, true, true, true, true, true, false, false, false, false, false },{ true, true, true, true, false, true, true, true, true, true, false, false, false, false, false },{ true, true, true, true, false, true, true, true, true, true, false, false, false, false, false },{ true, true, true, true, false, true, true, true, true, true, false, false, true, true, false },{ true, true, true, true, false, true, true, true, true, true, false, true, true, true, true },{ true, true, true, true, false, true, true, true, true, true, false, true, true, true, true },{ true, true, true, true, true, true, true, true, true, true, false, true, true, true, true },{ false, true, true, true, true, true, true, true, true, true, false, true, true, true, true },{ false, false, true, true, true, true, true, true, true, true, false, true, true, true, true },{ false, false, false, true, true, true, true, true, true, true, false, true, true, true, true },{ false, false, false, false, false, true, true, true, true, true, true, true, true, true, true },{ false, false, false, false, false, true, true, true, true, true, true, true, true, true, false },{ false, false, false, false, false, true, true, true, true, true, true, true, true, true, false },{ false, false, false, false, false, true, true, true, true, true, true, true, true, false, false },{ false, false, false, false, false, true, true, true, true, true, false, false, false, false, false },{ false, false, false, false, false, true, true, true, true, true, false, false, false, false, false },{ false, false, false, false, false, true, true, true, true, true, false, false, false, false, false },{ false, false, false, false, false, true, true, true, true, true, false, false, false, false, false },{ false, false, false, false, false, true, true, true, true, true, false, false, false, false, false },{ false, false, false, false, false, true, true, true, true, true, false, false, false, false, false },{ false, false, false, false, false, true, true, true, true, true, false, false, false, false, false } };


void gameDisp() {

}

void game() {
    int offset = 0;
    int height = 40;
    int duration = 500;
    uint32_t time = 0;
    bool lock = false;
    bool dinostate = false;
    uint32_t swaptime = to_ms_since_boot(get_absolute_time());
    std::srand(std::time(nullptr));

    std::vector<float> cacti;
    cacti.push_back(200);
    cacti.push_back(cacti.at(0) + (std::rand() % 120) + 150);
    cacti.push_back(cacti.at(1) + (std::rand() % 120) + 150);

    bool lost = false;
    int score = 0;
    bool scorelock = false;
    float speed = 1.0;
    while (true) {

        if (!lost) {
            ssd1306_clear(&disp);
            speed = 1.0 + ((float)score / 25.0);

            for (int i = 0; i < 3; i++) {
                cacti[i] = cacti.at(i) - (4 + speed);
            }

            if (cacti.at(0) < -50) {
                cacti[0] = cacti.at(1);
                cacti[1] = cacti.at(2);
                cacti[2] = (cacti.at(1) + (std::rand() % 100) + (140 + (10 * speed)));
                scorelock = false;

            }

            if (cacti.at(0) <= 5 && !scorelock) {
                score++;

                scorelock = true;
            }

            if ((!gpio_get(rot_sw) || !gpio_get(dit_pin) || !gpio_get(dah_pin)) && !lock) {
                lock = true;
                time = to_ms_since_boot(get_absolute_time());
            }
            if ((to_ms_since_boot(get_absolute_time()) - time) < duration && lock) {
                offset = ((to_ms_since_boot(get_absolute_time()) - time) % duration);
                offset = (int)((-1.0 / ((duration / height) * (duration * 0.25))) * (offset) * (offset - duration));
            } else {
                offset = 0;
                lock = false;
            }
            if (to_ms_since_boot(get_absolute_time()) - swaptime > 100) {
                swaptime = to_ms_since_boot(get_absolute_time());
                dinostate = !dinostate;
            }
            for (int i = 0; i < 3; i++) {

                for (int x = 0; x < 15; x++) {
                    for (int y = 0; y < 24; y++) {
                        if (cactus[y][x]) {
                            ssd1306_draw_pixel(&disp, x + cacti.at(i), y + abs(24 - 64));
                        }

                    }
                }
            }
            for (int x = 0; x < 20; x++) {
                for (int y = 0; y < 21; y++) {
                    if ((dinostate && dino[y][x + 20]) || (!dinostate && dino[y][x])) {
                        if (dispbuf[x + 10][y + abs(21 + offset - 64)]) {
                            lost = true;
                            lock = false;
                        } else {
                            ssd1306_draw_pixel(&disp, x + 10, y + abs(21 + offset - 64));
                        }
                    }
                }
            }
            ssd1306_draw_string_with_font(&disp, 0, 0, 2, font_8x5, std::format("{}", score).c_str());
            ssd1306_draw_string_with_font(&disp, 128 - (std::format("{}", highScore).length() * 12), 0, 2, font_8x5, std::format("{}", highScore).c_str());

        } else {
            if (score > highScore) {
                highScore = score;

                saveSettings(false);
            }
            ssd1306_draw_string_with_font(&disp, (128 - (10 * 8)) / 2, (64 - (16)) / 2, 2, font_8x5, std::format("YOU LOSE", score).c_str());
            if (!lock && (!gpio_get(rot_sw) || !gpio_get(dit_pin) || !gpio_get(dah_pin))) {
                lock = true;
            } if (lock && (gpio_get(rot_sw) && gpio_get(dit_pin) && gpio_get(dah_pin))) {
                return;
            }
        }
        ssd1306_show(&disp);

    }
}

