#include "display.h"
#include <string>
#include <format>
#include <string_view>
#include <sstream>

ssd1306_t disp;

void drawMain() {

    ssd1306_clear(&disp);
    std::string status = std::format("{} WPM ", speed);
    ssd1306_draw_string_with_font(&disp, 128 - ((status.length() * 5) + (status.length() - 1)), 2, 1, font_8x5, status.c_str());

    ssd1306_draw_string_with_font(&disp, 0, 1, 1, font_8x5, " V1.1");
    ssd1306_draw_inverted_square(&disp, 0, 0, 129, 10);

    std::string arr[] = { "", "", "", "", "" };
    uart_puts(uart0, std::format("{} {}\n", decode.length(), decode.length() / 22).c_str());

    if (decode.length() <= 21) {
        arr[0] = decode;
    } else if (decode.length() <= 42) {
        arr[0] = decode.substr(0, 21);
        arr[1] = decode.substr(21, 21);
    } else if (decode.length() <= 63) {
        arr[0] = decode.substr(0, 21);
        arr[1] = decode.substr(21, 21);
        arr[2] = decode.substr(42, 21);
    } else if (decode.length() <= 84) {
        arr[0] = decode.substr(0, 21);
        arr[1] = decode.substr(21, 21);
        arr[2] = decode.substr(42, 21);
        arr[3] = decode.substr(63, 21);
    } else if (decode.length() <= 105) {
        arr[0] = decode.substr(0, 21);
        arr[1] = decode.substr(21, 21);
        arr[2] = decode.substr(42, 21);
        arr[3] = decode.substr(63, 21);
        arr[4] = decode.substr(84, 21);
    } else {
        decode = decode.substr(decode.length() - 105, 105);
        arr[0] = decode.substr(0, 21);
        arr[1] = decode.substr(21, 21);
        arr[2] = decode.substr(42, 21);
        arr[3] = decode.substr(63, 21);
        arr[4] = decode.substr(84, 21);
    }


    ssd1306_draw_string_with_font(&disp, 0, 2 + (10 * 1), 1, font_8x5, arr[0].c_str());
    ssd1306_draw_string_with_font(&disp, 0, 2 + (10 * 2), 1, font_8x5, arr[1].c_str());
    ssd1306_draw_string_with_font(&disp, 0, 2 + (10 * 3), 1, font_8x5, arr[2].c_str());
    ssd1306_draw_string_with_font(&disp, 0, 2 + (10 * 4), 1, font_8x5, arr[3].c_str());
    ssd1306_draw_string_with_font(&disp, 0, 2 + (10 * 5), 1, font_8x5, arr[4].c_str());

    ssd1306_show(&disp);
}

void drawMenu() {
    std::stringstream str;
    ssd1306_clear(&disp);
    bool first = 0;
    int offset = selected_item - 3;
    if (offset <= 0) {
        offset = 0;
        ssd1306_draw_string_with_font(&disp, 4, 4 + (15 * 0), 1, font_8x5, "<- Back");
        first = true;
    }

    for (int i = 0; i < optionList.size() && i < 4 - first; i++) {

        ssd1306_draw_string_with_font(&disp, 4, 4 + (15 * (i + first)), 1, font_8x5, optionList.at(i + offset - !first).name.c_str());
        if (optionList.at(i + offset - !first).useValueIndex) {
            ssd1306_draw_string_with_font(&disp, 124 - ((optionList.at(i + offset - !first).arr.at(optionList.at(i + offset - !first).valueIndex).length() * 5) + (optionList.at(i + offset - !first).arr.at(optionList.at(i + offset - !first).valueIndex).length() - 1)), 4 + (15 * (i + first)), 1, font_8x5, optionList.at(i + offset - !first).arr.at(optionList.at(i + offset - !first).valueIndex).c_str());
        } else if (optionList.at(i + offset - !first).useBool) {
            if (!optionList.at(i + offset - !first).value) {
                ssd1306_draw_empty_square(&disp, 115, 3 + (15 * (i + first)), 8, 8);
            } else {
                ssd1306_draw_square(&disp, 115, 3 + (15 * (i + first)), 9, 9);

            }

        } else {
            str << std::vformat(optionList.at(i + offset - !first).format, std::make_format_args(optionList.at(i + offset - !first).value));
            ssd1306_draw_string_with_font(&disp, 124 - ((str.str().length() * 5) + (str.str().length() - 1)), 4 + (15 * (i + first)), 1, font_8x5, str.str().c_str());
        }
        str.str("");
        str.clear();


    }
    ssd1306_draw_empty_square(&disp, 0, 1 + (15 * (selected_item - offset)), 127, 12);
    if (clicked_item != -1) {
        ssd1306_draw_inverted_square(&disp, 1, 2 + (15 * (selected_item - offset)), 126, 11);
    }

    ssd1306_show(&disp);
}
