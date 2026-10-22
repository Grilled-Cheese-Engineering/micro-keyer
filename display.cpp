#include "display.h"
#include <string>
#include <format>
#include <string_view>
#include <sstream>
#include "hardware/adc.h"

ssd1306_t disp;

void drawMain() {

    ssd1306_clear(&disp);
    std::string status = std::format("{} WPM ", speed);

    gpio_pull_up(19);
    sleep_ms(2);
    float v = 0.0;
    int x = 0;
    for (int i = 0; i < 50; i++) {
        x += adc_read();
    }
    gpio_pull_down(19);

    x /= 50;

    v = (((x * 2.0) / 1243.24324) - 3) * 83.33;

    std::string voltage = std::format(" {:.2f}% ", v);
    ssd1306_draw_string_with_font(&disp, 128 - ((voltage.length() * 5) + (voltage.length() - 1)) - ((status.length() * 5) + (status.length() - 1)) - 8, 2, 1, font_8x5, voltage.c_str());

    ssd1306_draw_string_with_font(&disp, 128 - ((status.length() * 5) + (status.length() - 1)), 2, 1, font_8x5, status.c_str());

    ssd1306_draw_string_with_font(&disp, 0, 1, 1, font_8x5, " V1.2");
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
    int offset = selected_item - 3;
    if (offset <= 0) {
        offset = 0;
    }
    for (int i = 0; i + offset < optionList.size() && i < 5; i++) {

        ssd1306_draw_string_with_font(&disp, 4, 5 + (15 * i), 1, font_8x5, optionList.at(i + offset).name.c_str());
        if (optionList.at(i + offset).useValueIndex) {
            ssd1306_draw_string_with_font(&disp, 124 - ((optionList.at(i + offset).arr.at(optionList.at(i + offset).valueIndex).length() * 5) + (optionList.at(i + offset).arr.at(optionList.at(i + offset).valueIndex).length() - 1)), 5 + (15 * i), 1, font_8x5, optionList.at(i + offset).arr.at(optionList.at(i + offset).valueIndex).c_str());
        } else if (optionList.at(i + offset).useBool) {
            ssd1306_draw_empty_square(&disp, 115, 4 + (15 * i), 8, 8);

            if (optionList.at(i + offset).value) {
                ssd1306_draw_square(&disp, 117, 6 + (15 * i), 5, 5);
            }
        } else if (!optionList.at(i + offset).useAction) {

            str << std::vformat(optionList.at(i + offset).format, std::make_format_args(optionList.at(i + offset).value));
            ssd1306_draw_string_with_font(&disp, 124 - ((str.str().length() * 5) + (str.str().length() - 1)), 5 + (15 * i), 1, font_8x5, str.str().c_str());
        }
        str.str("");
        str.clear();


    }
    ssd1306_draw_empty_square(&disp, 0, 2 + (15 * (selected_item - offset)), 127, 12);
    if (clicked_item > 0 && !optionList.at(clicked_item).useBool) {
        ssd1306_draw_inverted_square(&disp, 1, 3 + (15 * (selected_item - offset)), 126, 11);
    }

    ssd1306_show(&disp);
}

void drawFlashMode() {
    ssd1306_clear(&disp);
    ssd1306_draw_string_with_font(&disp, 8 + (128 / 2) - ((13 * 7) / 2), 32 - (8 / 2), 1, font_8x5, "Flashing Mode");
    ssd1306_show(&disp);
}