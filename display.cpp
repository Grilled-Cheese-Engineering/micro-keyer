#include "display.h"
#include <string>
#include <format>
#include <string_view>

ssd1306_t disp;

void drawMain() {
    ssd1306_clear(&disp);
    std::string status = std::format("{} WPM", speed);
    ssd1306_draw_string_with_font(&disp, 128 - ((status.length() + 1) * 5), 0, 1, font_8x5, status.c_str());
    ssd1306_show(&disp);
}