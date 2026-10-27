#include <iostream>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "hardware/i2c.h"
#include "pico/time.h"
#include "hardware/uart.h"
#include "hardware/pwm.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"
#include "chars.h"
#include "sstream"
#include <format>
#include <string_view>

#include "bsp/board_api.h"
#include "tusb.h"

#include "usb_descriptors.h"

#include "display.h"

const uint8_t* contents;
uint32_t saved_interrupts;

int dit_state;
int dah_state;

int last = -1;
int curent = -1;
int next = -1;

int speed;
int tone;
int mute;

int level;
int basetime;

int keyerMode;
bool USBMode[3];

bool dumbMIDI;

bool reverse;

bool rotlock = false;
bool swlock = false;
bool cllock = false;
std::string str;

std::vector<int> elements;
uint32_t lastChar = 0;
uint32_t straightChar = 0;
uint32_t rotPress = 0;

bool recordMode = false;
std::vector<int> recordArr;
bool hasSpace = true;
bool recordLock = false;
bool usbState = false;
uint8_t keycode[6] = { 0 };

uint8_t msg[4];

int selected_item = -1;
int clicked_item = -1;

std::string decode;

std::vector<MenuOption> optionList;

settings options;

std::string playbackStr;

float voltage;

bool lastDit = false;
bool lastDah = false;

void print(std::string str);
void playStr();


std::string decodeChar(std::vector<int> elements) {
    std::stringstream stream;
    bool match = true;
    for (int i = 0; i < chars.size(); i++) {
        if (chars.at(i).size() == elements.size()) {
            for (int x = 0; x < elements.size(); x++) {
                if (elements.at(x) != chars.at(i).at(x)) {
                    match = false;
                    break;
                }
            }
        } else {
            match = false;
        }
        if (match) {
            stream << strs.at(i);
            return stream.str();
        }
        match = true;
    }
    return "_";
}

void sendKey(std::string s) {
    if (tud_suspended()) {
        tud_remote_wakeup();
    }
    if (s == " ") {
        keycode[0] = HID_KEY_SPACE;
    } else {
        uint8_t conv_table[128][2] = { HID_ASCII_TO_KEYCODE };
        keycode[0] = conv_table[(int)*s.c_str()][1];
    }
}

void key(bool x) {
    if (x) {
        pwm_set_gpio_level(pwm_pin, level);
        gpio_pull_up(radio_pin);
        if (USBMode[2] && !dumbMIDI) {
            msg[0] = 0x09; // Note On - Channel 1
            msg[1] = 0x90; // Note Number
            msg[2] = 2;
            msg[3] = 0x7F; // Velocity
            tud_midi_n_stream_write(0, 0, msg, 4);
        }
    } else {
        pwm_set_gpio_level(pwm_pin, 0);
        gpio_pull_down(radio_pin);
        if (USBMode[2] && !dumbMIDI) {
            msg[0] = 0x08; // Note On - Channel 1
            msg[1] = 0x80; // Note Number
            msg[2] = 2;
            msg[3] = 0x0; // Velocity
            tud_midi_n_stream_write(0, 0, msg, 4);
        }
    }
    if (USBMode[2]) {
        tud_task();
    }
}

int64_t coolDown(alarm_id_t id, void* user_data) {
    curent = -1;
    return 0;
}

int64_t turnOff(alarm_id_t id, void* user_data) {
    key(false);
    add_alarm_in_ms(basetime, *coolDown, user_data, false);
    lastChar = to_ms_since_boot(get_absolute_time());
    elements.push_back(*((int*)user_data));
    if (recordMode) {
        recordArr.push_back(*((int*)user_data));
    }

    return 0;
}

void doDit() {
    key(true);
    add_alarm_in_ms(basetime, *turnOff, (void*)&dit, false);
    hasSpace = false;
    curent = dit;
    last = dit;
}

void doDah() {
    key(true);
    add_alarm_in_ms(basetime * 3, *turnOff, (void*)&dah, false);
    hasSpace = false;
    curent = dah;
    last = dah;
}

int main() {
    stdio_init_all();
    adc_init();
    loadSettings();
    uart_init(uart0, 115200);
    gpio_set_function(uart_tx_pin, GPIO_FUNC_UART);
    gpio_set_function(uart_rx_pin, GPIO_FUNC_UART);

    gpio_set_function(pwm_pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pwm_pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv_int_frac(&config, 125, 0);
    pwm_config_set_wrap(&config, (125000000 / 125) / (tone * !mute));
    pwm_init(slice_num, &config, true);
    key(true);
    sleep_ms(100);
    key(false);

    board_init();
    tusb_init();

    if (USBMode[1]) {
        const tusb_rhport_init_t rh_init = {
            .role = TUSB_ROLE_DEVICE,
            .speed = TUD_OPT_HIGH_SPEED ? TUSB_SPEED_HIGH : TUSB_SPEED_FULL
        };
        TU_ASSERT(tud_rhport_init(BOARD_TUD_RHPORT, &rh_init));
    }

    adc_gpio_init(batt_adc_pin);
    adc_select_input(3);

    voltage = (adc_read() * 2) * (3.3f / (1 << 12));


    gpio_init(batt_adc_en_pin);
    gpio_set_dir(batt_adc_en_pin, GPIO_IN);
    gpio_pull_down(batt_adc_en_pin);

    gpio_init(dit_pin);
    gpio_set_dir(dit_pin, GPIO_IN);
    gpio_pull_up(dit_pin);

    gpio_init(dah_pin);
    gpio_set_dir(dah_pin, GPIO_IN);
    gpio_pull_up(dah_pin);

    gpio_init(radio_pin);
    gpio_set_dir(radio_pin, GPIO_IN);
    gpio_pull_down(radio_pin);

    gpio_init(rot_a);
    gpio_set_dir(rot_a, GPIO_IN);
    gpio_pull_up(rot_a);

    gpio_init(rot_b);
    gpio_set_dir(rot_b, GPIO_IN);
    gpio_pull_up(rot_b);

    gpio_init(rot_sw);
    gpio_set_dir(rot_sw, GPIO_IN);
    gpio_pull_up(rot_sw);

    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(display_sda, GPIO_FUNC_I2C);
    gpio_set_function(display_scl, GPIO_FUNC_I2C);
    gpio_pull_up(display_sda);
    gpio_pull_up(display_scl);

    disp.external_vcc = false;
    ssd1306_init(&disp, 128, 64, 0x3C, I2C_PORT);
    drawMain();



    optionList.push_back(MenuOption("<- Back",
        [](MenuOption& self) {clicked_item = -1; selected_item = -1; drawMain();}
    ));

    optionList.push_back(MenuOption("Speed", "{} WPM", speed, 100, 0,
        [](MenuOption& self) {self.value += 1; setSpeed(self.value);},
        [](MenuOption& self) {if (self.value > 1) { self.value -= 1; setSpeed(self.value); }},
        [](MenuOption& self) {setSpeed(self.value);},
        [](MenuOption& self) {self.value = speed;}
    ));


    optionList.push_back(MenuOption("Tone", "{} Hz", tone, 1000, 0,
        [](MenuOption& self) {self.value += 10; setTone(self.value);},
        [](MenuOption& self) {if (self.value > 10) { self.value -= 10; setTone(self.value); }},
        [](MenuOption& self) {setTone(self.value);},
        [](MenuOption& self) {self.value = tone;}

    ));

    optionList.push_back(MenuOption("Mode", std::vector<std::string>{"Iambic A", "Iambic B", "Straight key", "Bug Emu"}, keyerMode,
        [](MenuOption& self) {if (self.valueIndex < 3) { self.valueIndex++; setKeyerMode(self.valueIndex); }},
        [](MenuOption& self) {if (self.valueIndex > 0) { self.valueIndex--; setKeyerMode(self.valueIndex); }},
        [](MenuOption& self) {setKeyerMode(self.valueIndex);},
        [](MenuOption& self) {self.valueIndex = keyerMode;}
    ));

    optionList.push_back(MenuOption("Reverse", reverse,
        [](MenuOption& self) {self.value = !self.value; reverse = self.value; if (self.value) { dit_pin = 27;dah_pin = 26; } else { dit_pin = 26;dah_pin = 27; }saveSettings();},
        [](MenuOption& self) {self.value = reverse;}
    ));

    optionList.push_back(MenuOption("Mute", mute,
        [](MenuOption& self) {self.value = !self.value; mute = self.value; setTone(tone);},
        [](MenuOption& self) {self.value = mute;}
    ));

    optionList.push_back(MenuOption("Serial", USBMode[0],
        [](MenuOption& self) {self.value = !self.value; USBMode[0] = self.value; saveSettings();},
        [](MenuOption& self) {self.value = USBMode[0];}
    ));

    optionList.push_back(MenuOption("HID", USBMode[1],
        [](MenuOption& self) {self.value = !self.value; USBMode[1] = self.value; saveSettings();},
        [](MenuOption& self) {self.value = USBMode[1];}
    ));

    optionList.push_back(MenuOption("MIDI", USBMode[2],
        [](MenuOption& self) {self.value = !self.value; USBMode[2] = self.value; saveSettings();},
        [](MenuOption& self) {self.value = USBMode[2];}
    ));

    optionList.push_back(MenuOption("Dumb MIDI", dumbMIDI,
        [](MenuOption& self) {self.value = !self.value; dumbMIDI = self.value; saveSettings();},
        [](MenuOption& self) {self.value = dumbMIDI;}
    ));

    optionList.push_back(MenuOption("Reset Settings    ->",
        [](MenuOption& self) {reset(true);for (int i = 0; i < optionList.size(); i++) { optionList.at(i).load(); }drawMenu();}
    ));

    optionList.push_back(MenuOption("Flash Firmware    ->",
        [](MenuOption& self) {drawFlashMode(); reset_usb_boot(0, 0);}
    ));

    dit_state = !gpio_get(dit_pin);
    dah_state = !gpio_get(dah_pin);
    lastDit = dit_state;
    lastDah = dah_state;

    while (true) {

        tud_task();
        if (playbackStr.length() > 0) {
            playStr();
        }
        dit_state = !gpio_get(dit_pin);
        dah_state = !gpio_get(dah_pin);
        if (USBMode[2] && dumbMIDI) {
            if (dit_state && !lastDit) {
                msg[0] = 0x09;
                msg[1] = 0x90;
                msg[2] = 0;
                msg[3] = 0x7F;
                tud_midi_n_stream_write(0, 0, msg, 4);
            } else if (!dit_state && lastDit) {
                msg[0] = 0x08;
                msg[1] = 0x80;
                msg[2] = 0;
                msg[3] = 0x0;
                tud_midi_n_stream_write(0, 0, msg, 4);
            }
            lastDit = dit_state;

            if (dah_state && !lastDah) {
                msg[0] = 0x09;
                msg[1] = 0x90;
                msg[2] = 1;
                msg[3] = 0x7F;
                tud_midi_n_stream_write(0, 0, msg, 4);
            } else if (!dah_state && lastDah) {
                msg[0] = 0x08;
                msg[1] = 0x80;
                msg[2] = 1;
                msg[3] = 0x0;
                tud_midi_n_stream_write(0, 0, msg, 4);
            }
            lastDah = dah_state;
            tud_task();
        } else {
            switch (keyerMode) {
            case 2:
                if (curent != 1 && (dit_state || dah_state)) {
                    key(true);
                    straightChar = to_ms_since_boot(get_absolute_time());
                    curent = 1;
                    hasSpace = false;
                } else if (curent == 1 && !dit_state && !dah_state && to_ms_since_boot(get_absolute_time()) - straightChar > 20) {
                    key(false);
                    print(std::format("\n{}", (to_ms_since_boot(get_absolute_time()) - straightChar)));
                    if (to_ms_since_boot(get_absolute_time()) - straightChar > basetime * 0.4 && to_ms_since_boot(get_absolute_time()) - straightChar < basetime * 2.4) {
                        elements.push_back(dit);
                    } else if (to_ms_since_boot(get_absolute_time()) - straightChar > basetime * 2.4 && to_ms_since_boot(get_absolute_time()) - straightChar < basetime * 6) {
                        elements.push_back(dah);
                    }
                    curent = -1;
                    lastChar = to_ms_since_boot(get_absolute_time());

                }

                if (to_ms_since_boot(get_absolute_time()) - lastChar > basetime * 7 && !hasSpace && curent == -1) {
                    print(" ");
                    if (USBMode[1]) {
                        sendKey(" ");
                    }
                    decode += " ";
                    if (selected_item == -1) {
                        drawMain();
                    }
                    hasSpace = true;
                    if (recordMode) {
                        recordArr.pop_back();
                        recordArr.push_back(space);
                    }
                } else if (elements.size() > 0 && to_ms_since_boot(get_absolute_time()) - lastChar > basetime * 2.5 && curent == -1) {
                    print("\n" + decodeChar(elements));
                    if (USBMode[1]) {
                        sendKey(decodeChar(elements));
                    }
                    decode += decodeChar(elements);
                    if (selected_item == -1) {
                        drawMain();
                    }
                    elements.clear();
                    if (recordMode && recordArr.size() > 0) {
                        recordArr.push_back(gap);
                    }
                }

                break;
            case 3:
                if (curent == -1 && dit_state) {
                    doDit();
                }
                if (dah_state && curent == -1) {
                    key(true);
                    if (curent != dah) {
                        straightChar = to_ms_since_boot(get_absolute_time());
                    }
                    curent = dah;

                    hasSpace = false;
                } else if (!dah_state && (curent == dah || curent == 3)) {
                    if (to_ms_since_boot(get_absolute_time()) - straightChar > 5) {
                        key(false);
                        if (to_ms_since_boot(get_absolute_time()) - straightChar > basetime * 0.4 && to_ms_since_boot(get_absolute_time()) - straightChar < basetime * 2.4) {
                            elements.push_back(dit);
                        } else if (to_ms_since_boot(get_absolute_time()) - straightChar > basetime * 2.4 && to_ms_since_boot(get_absolute_time()) - straightChar < basetime * 6) {
                            elements.push_back(dah);
                        }
                        curent = -1;
                        lastChar = to_ms_since_boot(get_absolute_time());
                    }
                }

                if (to_ms_since_boot(get_absolute_time()) - lastChar > basetime * 7 && !hasSpace && curent == -1) {
                    print(" ");
                    if (USBMode[1]) {
                        sendKey(" ");
                    }
                    decode += " ";
                    if (selected_item == -1) {
                        drawMain();
                    }
                    hasSpace = true;
                    if (recordMode) {
                        recordArr.pop_back();
                        recordArr.push_back(space);
                    }
                } else if (elements.size() > 0 && to_ms_since_boot(get_absolute_time()) - lastChar > basetime * 2.5 && curent == -1) {
                    print("\n" + decodeChar(elements));
                    if (USBMode[1]) {
                        sendKey(decodeChar(elements));
                    }
                    decode += decodeChar(elements);
                    if (selected_item == -1) {
                        drawMain();
                    }
                    elements.clear();
                    if (recordMode && recordArr.size() > 0) {
                        recordArr.push_back(gap);
                    }
                }
                break;
            case 1:
                if (dah_state && dit_state) {
                    next = 3;
                } else if (dit_state && !dah_state && next != 3) {
                    if (curent == dah || curent == -1) {
                        next = dit;
                    }
                } else if (dah_state && !dit_state && next != 3) {
                    if (curent == dit || curent == -1) {
                        next = dah;
                    }
                }

                if (curent == -1) {
                    if (next == dit) {
                        doDit();
                        next = -1;
                    } else if (next == dah) {
                        doDah();
                        next = -1;
                    } else if (next == 3) {
                        if (last == dit) {
                            doDah();
                        } else if (last == dah) {
                            doDit();
                        }
                        next = -1;
                    }
                }

                if (to_ms_since_boot(get_absolute_time()) - lastChar > basetime * 7 && !hasSpace && curent == -1) {
                    print(" ");
                    if (USBMode[1]) {
                        sendKey(" ");
                    }
                    decode += " ";
                    if (selected_item == -1) {
                        drawMain();
                    }
                    hasSpace = true;
                    if (recordMode) {
                        recordArr.pop_back();
                        recordArr.push_back(space);
                    }
                } else if (elements.size() > 0 && to_ms_since_boot(get_absolute_time()) - lastChar > basetime * 2.5 && curent == -1) {
                    print(decodeChar(elements));
                    if (USBMode[1]) {
                        sendKey(decodeChar(elements));
                    }
                    decode += decodeChar(elements);
                    if (selected_item == -1) {
                        drawMain();
                    }
                    elements.clear();
                    if (recordMode && recordArr.size() > 0) {
                        recordArr.push_back(gap);
                    }
                }

                break;
            case 0:

                if (curent == -1) {
                    if (last == dit && dit_state && dah_state) {
                        doDit();
                    } else if (last == dah && dit_state && dah_state) {
                        doDah();
                    } else if (dit_state) {
                        doDit();
                    } else if (dah_state) {
                        doDah();
                    }
                }
                if (to_ms_since_boot(get_absolute_time()) - lastChar > basetime * 7 && !hasSpace && curent == -1) {
                    print(" ");
                    if (USBMode[1]) {
                        sendKey(" ");
                    }
                    decode += " ";
                    if (selected_item == -1) {
                        drawMain();
                    }
                    hasSpace = true;
                    if (recordMode) {
                        recordArr.pop_back();
                        recordArr.push_back(space);
                    }
                } else if (elements.size() > 0 && to_ms_since_boot(get_absolute_time()) - lastChar > basetime * 2.5 && curent == -1) {
                    print(decodeChar(elements));
                    if (USBMode[1]) {
                        sendKey(decodeChar(elements));
                    }
                    decode += decodeChar(elements);
                    if (selected_item == -1) {
                        drawMain();
                    }
                    elements.clear();
                    if (recordMode && recordArr.size() > 0) {
                        recordArr.push_back(gap);
                    }
                }

                break;
            default:
                break;
            }
        }

        if (!gpio_get(rot_a) && gpio_get(rot_b) && !rotlock) {
            rotlock = true;
            if (selected_item == -1) {
                setSpeed(speed + 1);
                drawMain();
            } else {
                if (clicked_item > -1) {
                    optionList.at(clicked_item).turnR();
                } else {
                    if (selected_item < optionList.size() - 1) {
                        selected_item++;
                        print(std::format("selected {} clicked {}\n", selected_item, clicked_item));
                    }
                }
                drawMenu();
            }
            sleep_ms(50);

        }
        if (gpio_get(rot_a) && !gpio_get(rot_b) && !rotlock) {
            rotlock = true;
            if (selected_item == -1) {
                if (speed > 1) {
                    setSpeed(speed - 1);
                    drawMain();
                }
            } else {
                if (clicked_item > -1) {
                    optionList.at(clicked_item).turnL();
                } else {
                    if (selected_item > 0) {
                        selected_item--;
                        print(std::format("selected {} clicked {}\n", selected_item, clicked_item));
                    }
                }
                drawMenu();
            }
            sleep_ms(50);

        }
        if (rotlock && gpio_get(rot_a) && gpio_get(rot_b)) {
            rotlock = false;
        }

        if (!swlock && !gpio_get(rot_sw)) {
            swlock = true;
            rotPress = to_ms_since_boot(get_absolute_time());
            cllock = false;
        }

        if (swlock && gpio_get(rot_sw)) {
            swlock = false;
            if (!cllock) {
                if (selected_item == -1) {
                    selected_item = 1;
                    print(std::format("selected {} clicked {}\n", selected_item, clicked_item));
                    for (int i = 0; i < optionList.size(); i++) {
                        optionList.at(i).load();
                    }
                    drawMenu();
                } else {
                    if (clicked_item != -1) {
                        clicked_item = -1;
                        optionList.at(selected_item).click();
                        print(std::format("save\n"));
                        saveSettings();

                        drawMenu();
                    } else {
                        if (optionList.at(selected_item).useBool) {
                            optionList.at(selected_item).select();
                            drawMenu();
                        } else if (optionList.at(selected_item).useAction) {
                            optionList.at(selected_item).select();
                        } else {
                            clicked_item = selected_item;
                            drawMenu();
                        }


                    }
                    print(std::format("selected {} clicked {}\n", selected_item, clicked_item));
                }
            }

        }
        if (swlock && !gpio_get(rot_sw) && to_ms_since_boot(get_absolute_time()) - rotPress > 1000 && !cllock) {
            cllock = true;
            decode = "";
            drawMain();
        }
        if (USBMode[1]) {
            if (tud_hid_ready()) {
                if (!usbState && keycode[0] > 0) {
                    tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
                    usbState = true;
                } else if (usbState) {

                    keycode[0] = 0;
                    tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
                    usbState = false;
                }
            }
        }
    }
}


void setSpeed(int x) {
    if (x > 0) {
        speed = x;
        basetime = 1200 / speed;
        print(std::format("change speed {}\n", speed));
        saveSettings();
    }

}

void setTone(int x) {
    if (x > 0) {
        tone = x;
        level = ((125000000 / 125) / (tone * !mute)) / 2;
        pwm_set_wrap(pwm_gpio_to_slice_num(pwm_pin), (125000000 / 125) / (tone * !mute));
        saveSettings();
    }
}

void setKeyerMode(int x) {
    if (x > -1) {
        keyerMode = x;
        saveSettings();
    }
}

void reset(bool screen) {
    speed = 20;
    tone = 340;
    mute = 0;
    keyerMode = 1;
    USBMode[0] = 1;
    USBMode[1] = 0;
    USBMode[2] = 1;
    dumbMIDI = 0;
    reverse = 0;
    ssd1306_clear(&disp);
    ssd1306_draw_string_with_font(&disp, 8 + (128 / 2) - ((13 * 7) / 2), 32 - (8 / 2), 1, font_8x5, "Settings Reset");
    ssd1306_show(&disp);
    sleep_ms(2000);
    saveSettings();
}
void loadSettings() {
    const settings* opts = (const settings*)(XIP_BASE + FLASH_TARGET_OFFSET);

    if (opts->speed == -1 || opts->tone == -1 || opts->mute == -1 || opts->keyerMode == -1) {
        reset(false);
    } else {
        speed = opts->speed;
        tone = opts->tone;
        mute = opts->mute;
        keyerMode = opts->keyerMode;
        USBMode[0] = opts->USBMode[0];
        USBMode[1] = opts->USBMode[1];
        USBMode[2] = opts->USBMode[2];
        dumbMIDI = opts->dumbMIDI;
        reverse = opts->reverse;

    }
    saveSettings();
    setTone(tone);
    setSpeed(speed);
}

void saveSettings() {
    options = { speed, tone, mute, keyerMode,{ USBMode[0],USBMode[1],USBMode[2] },dumbMIDI,reverse };
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, (const uint8_t*)&options, FLASH_PAGE_SIZE);
    restore_interrupts(ints);
}

void print(std::string str) {
    uart_puts(uart0, str.c_str());
    tud_cdc_n_write(0, (uint8_t const*)str.c_str(), str.length());
    tud_cdc_n_write_flush(0);
}

void playStr() {
    std::vector<int> elements;

    while (true) {
        elements.clear();
        for (int x = 0; x < strs.size(); x++) {
            if (strs[x] == playbackStr.substr(0, 1)) {
                elements = chars[x];
            }
        }
        if (elements.empty()) {
            decode += playbackStr.substr(0, 1);
            print(playbackStr.substr(0, 1));
            sleep_ms(basetime * 7);

        } else {
            for (int x = 0; x < elements.size(); x++) {
                if (elements[x] == dit) {
                    key(true);
                    sleep_ms(basetime);
                    key(false);
                    sleep_ms(basetime);
                } else if (elements[x] == dah) {
                    key(true);
                    sleep_ms(basetime * 3);
                    key(false);
                    sleep_ms(basetime);
                }

            }

            sleep_ms(basetime * 3);

            print(playbackStr.substr(0, 1));
            decode += playbackStr.substr(0, 1);
        }
        drawMain();

        playbackStr.erase(0, 1);
        if (playbackStr.length() == 0) {
            break;
        }
    }

    decode += " ";
    print(" ");
    sleep_ms(basetime * 7);
}

void tud_cdc_rx_cb(uint8_t itf) {
    char buf[CFG_TUD_CDC_RX_BUFSIZE] = { 0 };
    uint32_t count = tud_cdc_n_read(itf, buf, sizeof(buf));

    for (int i = 0; i < CFG_TUD_CDC_RX_BUFSIZE;i++) {
        buf[i] = std::toupper(buf[i]);
    }

    playbackStr += std::string(buf);

    // for (int i = 0; i < 50;i++) {
    //     uart_puts(uart0, std::format("{}", (uint8_t)buf[i]).c_str());
    //     uart_puts(uart0, "\n");
    // }
    // for (int i = 2; i < CFG_TUD_CDC_RX_BUFSIZE;i++) {
    //     buf[i - 2] = buf[i];
    // }

    // buf[CFG_TUD_CDC_RX_BUFSIZE - 1] = 0;
    // buf[CFG_TUD_CDC_RX_BUFSIZE - 2] = 0;

    // uart_puts(uart0, "\n");
    // uart_puts(uart0, buf);
    // uart_puts(uart0, "\n");
    // uart_puts(uart0, "\n");
    // tud_cdc_n_write(0, buf, CFG_TUD_CDC_RX_BUFSIZE);
    // tud_cdc_n_write_flush(0);
    // tud_cdc_n_write(0, (void*)0x1f, 1);
    // tud_cdc_n_write_flush(0);
}
