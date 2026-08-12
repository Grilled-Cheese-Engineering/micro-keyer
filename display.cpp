#include "display.h"
#include <string>
#include <format>
#include <string_view>

ssd1306_t disp;

void drawMain() {
    ssd1306_clear(&disp);
    std::string status = std::format("{} WPM", speed);
    ssd1306_draw_string_with_font(&disp, 128 - ((status.length() + 1) * 5), 0, 1, font_8x5, status.c_str());

    ssd1306_draw_square(&disp, 0, 0, 5 * 6, 9);
    ssd1306_draw_string_with_font(&disp, 0, 0, 1, font_8x5, "V1.0");


    ssd1306_show(&disp);
}

void drawMenu() {
    ssd1306_clear(&disp);
    ssd1306_draw_square(&disp, 0, 14 + (15 * selected_item), 128, 2);
    ssd1306_draw_string_with_font(&disp, 0, 4 + (15 * 0), 1, font_8x5, "<- Back");
    ssd1306_draw_string_with_font(&disp, 0, 4 + (15 * 1), 1, font_8x5, "<- a");
    ssd1306_draw_string_with_font(&disp, 0, 4 + (15 * 2), 1, font_8x5, "<- b");
    ssd1306_draw_string_with_font(&disp, 0, 4 + (15 * 3), 1, font_8x5, "<- b");


    ssd1306_show(&disp);
}