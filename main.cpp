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
int tonemod;

int level;
int basetime;

int keyerMode;
bool USBMode[3];

bool rotlock = false;
bool swlock = false;

std::string str;

std::vector<int> elements;
uint32_t lastChar = 0;

bool recordMode = false;
std::vector<int> recordArr;
bool hasSpace = true;
bool recordLock = false;
bool usbState = false;
uint8_t keycode[6] = { 0 };

uint8_t msg[4];

uint8_t buf[64] = { 0 };


int selected_item = -1;
int clicked_item = -1;

std::string decode;

std::vector<MenuOption> optionList;

settings options;


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
        if (USBMode[2]) {
            msg[0] = 0x09; // Note On - Channel 1
            msg[1] = 0x90; // Note Number
            msg[2] = 1;
            msg[3] = 0x7F; // Velocity
            tud_midi_n_stream_write(0, 0, msg, 4);
        }
    } else {
        pwm_set_gpio_level(pwm_pin, 0);
        if (USBMode[2]) {
            msg[0] = 0x08; // Note On - Channel 1
            msg[1] = 0x80; // Note Number
            msg[2] = 1;
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
    loadSettings();
    uart_init(uart0, 115200);
    gpio_set_function(uart_tx_pin, GPIO_FUNC_UART);
    gpio_set_function(uart_rx_pin, GPIO_FUNC_UART);

    gpio_set_function(pwm_pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pwm_pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv_int_frac(&config, 125, 0);
    pwm_config_set_wrap(&config, (125000000 / 125) / (tone * tonemod));
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
    gpio_init(dit_pin);
    gpio_set_dir(dit_pin, GPIO_IN);
    gpio_set_pulls(dit_pin, true, false);

    gpio_init(dah_pin);
    gpio_set_dir(dah_pin, GPIO_IN);
    gpio_set_pulls(dah_pin, true, false);

    gpio_init(rot_a);
    gpio_set_dir(rot_a, GPIO_IN);
    gpio_set_pulls(rot_a, true, false);

    gpio_init(rot_b);
    gpio_set_dir(rot_b, GPIO_IN);
    gpio_set_pulls(rot_b, true, false);

    gpio_init(rot_sw);
    gpio_set_dir(rot_sw, GPIO_IN);
    gpio_set_pulls(rot_sw, true, false);

    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(display_sda, GPIO_FUNC_I2C);
    gpio_set_function(display_scl, GPIO_FUNC_I2C);
    gpio_pull_up(display_sda);
    gpio_pull_up(display_scl);

    // gpio_init(recordPin);
    // gpio_set_dir(recordPin, GPIO_IN);
    // gpio_set_pulls(recordPin, true, false);

    // gpio_init(playPin);
    // gpio_set_dir(playPin, GPIO_IN);
    // gpio_set_pulls(playPin, true, false);

    disp.external_vcc = false;
    ssd1306_init(&disp, 128, 64, 0x3C, I2C_PORT);
    drawMain();
    if (USBMode[0] && tud_cdc_connected()) {
        tud_cdc_write_str("Hello from Pico!\r\n");
        tud_cdc_write_flush(); // Forces the data to send immediately
    }


    optionList.push_back(MenuOption("Speed", "{} WPM", speed, 100, 0,
        [](MenuOption& self) {self.value += 1; setSpeed(self.value);},
        [](MenuOption& self) {self.value -= 1; setSpeed(self.value);},
        [](MenuOption& self) {setSpeed(self.value);},
        [](MenuOption& self) {self.value = speed;}
    ));


    optionList.push_back(MenuOption("Tone", "{} Hz", tone, 1000, 0,
        [](MenuOption& self) {self.value += 10; setTone(self.value);},
        [](MenuOption& self) {self.value -= 10; setTone(self.value);},
        [](MenuOption& self) {setTone(self.value);},
        [](MenuOption& self) {self.value = tone;}

    ));
    optionList.push_back(MenuOption("Mute", !tonemod,
        [](MenuOption& self) {self.value = !self.value; tonemod = !self.value; setTone(tone);},
        [](MenuOption& self) {self.value = !self.value; tonemod = !self.value; setTone(tone);},
        [](MenuOption& self) {tonemod = !self.value; setTone(tone);},
        [](MenuOption& self) {self.value = !tonemod;}
    ));

    optionList.push_back(MenuOption("SERIAL", USBMode[0],
        [](MenuOption& self) {self.value = !self.value; USBMode[0] = self.value;},
        [](MenuOption& self) {self.value = !self.value; USBMode[0] = self.value;},
        [](MenuOption& self) {USBMode[0] = self.value;saveSettings();},
        [](MenuOption& self) {self.value = USBMode[0];}
    ));

    optionList.push_back(MenuOption("HID", USBMode[1],
        [](MenuOption& self) {self.value = !self.value; USBMode[1] = self.value;},
        [](MenuOption& self) {self.value = !self.value; USBMode[1] = self.value;},
        [](MenuOption& self) {USBMode[1] = self.value;saveSettings();},
        [](MenuOption& self) {self.value = USBMode[1];}
    ));

    optionList.push_back(MenuOption("MIDI", USBMode[2],
        [](MenuOption& self) {self.value = !self.value; USBMode[2] = self.value;},
        [](MenuOption& self) {self.value = !self.value; USBMode[2] = self.value;},
        [](MenuOption& self) {USBMode[2] = self.value; saveSettings();},
        [](MenuOption& self) {self.value = USBMode[2];}
    ));

    while (true) {
        tud_task();
        dit_state = !gpio_get(dit_pin);
        dah_state = !gpio_get(dah_pin);
        if (dah_state && dit_state) {
            next = 3;
        } else if (dit_state && !dah_state) {
            if (curent == dah || curent == -1) {
                next = dit;
            }
        } else if (dah_state && !dit_state) {
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
            uart_puts(uart0, " ");
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
            uart_puts(uart0, decodeChar(elements).c_str());
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

        // if (!gpio_get(playPin)) {
        //     contents = (const uint8_t*)(XIP_BASE + FLASH_TARGET_OFFSET);

        //     for (int i = 0; i < FLASH_PAGE_SIZE; i++) {
        //         if (contents[i] == end) {
        //             hasSpace = false;
        //             break;
        //         } else if (contents[i] == dit) {
        //             key(true);
        //             sleep_ms(basetime);
        //             key(false);
        //             sleep_ms(basetime);
        //             elements.push_back(dit);
        //         } else if (contents[i] == dah) {
        //             key(true);
        //             sleep_ms(basetime * 3);
        //             key(false);
        //             sleep_ms(basetime);
        //             elements.push_back(dah);
        //         } else if (contents[i] == gap) {
        //             uart_puts(uart0, decodeChar(elements).c_str());
        //             elements.clear();
        //             sleep_ms(basetime * 3);
        //         } else if (contents[i] == space) {
        //             uart_puts(uart0, decodeChar(elements).c_str());
        //             elements.clear();
        //             uart_puts(uart0, " ");
        //             sleep_ms(basetime * 7);
        //         }
        //     }
        // }

        // if (gpio_get(recordPin)) {
        //     recordLock = false;
        // }
        // if (!gpio_get(recordPin) && !recordMode && !recordLock) {
        //     recordLock = true;
        //     recordMode = true;
        //     uart_puts(uart0, " start-- ");
        // } else if (!gpio_get(recordPin) && recordMode && !recordLock) {
        //     recordLock = true;
        //     recordMode = false;
        //     uart_puts(uart0, " --stop ");
        //     if (recordArr.size() != 0) {
        //         recordArr.pop_back();
        //         recordArr.push_back(end);

        //         alignas(4) uint8_t write_buf[FLASH_PAGE_SIZE];
        //         for (int i = 0; i < FLASH_PAGE_SIZE; i++) {
        //             write_buf[i] = 4;
        //         }

        //         for (int i = 0; i < recordArr.size(); i++) {
        //             write_buf[i] = recordArr.at(i);
        //         }

        //         saved_interrupts = save_and_disable_interrupts();
        //         flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
        //         flash_range_program(FLASH_TARGET_OFFSET, write_buf, FLASH_PAGE_SIZE);
        //         restore_interrupts(saved_interrupts);
        //         recordArr.clear();
        //     }
        // }
        //uart_puts(uart0, std::format("a {}, b {}, sw {}\n", gpio_get(rot_a), gpio_get(rot_b), gpio_get(rot_sw)).c_str());

        if (!gpio_get(rot_a) && gpio_get(rot_b) && !rotlock) {
            rotlock = true;
            if (selected_item == -1) {
                setSpeed(speed + 1);
                drawMain();
            } else {
                if (clicked_item > -1) {
                    optionList.at(clicked_item - 1).turnR();
                } else {
                    if (selected_item < optionList.size()) {
                        selected_item++;
                        uart_puts(uart0, std::format("selected {} clicked {}\n", selected_item, clicked_item).c_str());
                    }
                }
                drawMenu();
            }
            sleep_ms(50);

        }
        if (gpio_get(rot_a) && !gpio_get(rot_b) && !rotlock) {
            rotlock = true;
            if (selected_item == -1) {
                setSpeed(speed - 1);
                drawMain();
            } else {
                if (clicked_item > -1) {
                    optionList.at(clicked_item - 1).turnL();
                } else {
                    if (selected_item > 0) {
                        selected_item--;
                        uart_puts(uart0, std::format("selected {} clicked {}\n", selected_item, clicked_item).c_str());
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
        }
        if (swlock && gpio_get(rot_sw)) {
            // if (USBMode[0]) {
            //     save_boot_mode(MODE_MIDI);
            //     uart_puts(uart0, std::format("Change mode: MODE_MIDI\n").c_str());
            // } else if (USBMode[2]) {
            //     save_boot_mode(MODE_HID);
            //     uart_puts(uart0, std::format("Change mode: MODE_HID\n").c_str());
            // } else if (USBMode[1]) {
            //     save_boot_mode(MODE_CDC_SERIAL);
            //     uart_puts(uart0, std::format("Change mode: MODE_CDC_SERIAL\n").c_str());
            // }
            //
            if (selected_item == -1) {
                selected_item = 1;
                uart_puts(uart0, std::format("selected {} clicked {}\n", selected_item, clicked_item).c_str());
                for (int i = 0; i < optionList.size(); i++) {
                    optionList.at(i).load();
                }
                drawMenu();
            } else {
                if (clicked_item != -1) {
                    clicked_item = -1;
                    optionList.at(selected_item - 1).click();
                    uart_puts(uart0, std::format("save\n").c_str());
                    saveSettings();

                    drawMenu();
                } else {
                    clicked_item = selected_item;

                    drawMenu();
                    if (clicked_item == 0) {
                        clicked_item = -1;
                        selected_item = -1;

                        drawMain();
                    }

                }
                uart_puts(uart0, std::format("selected {} clicked {}\n", selected_item, clicked_item).c_str());
            }
            swlock = false;
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
    speed = x;
    basetime = 1200 / speed;
    uart_puts(uart0, std::format("change speed {}\n", speed).c_str());
    saveSettings();

}

void setTone(int x) {
    tone = x;
    level = ((125000000 / 125) / (tone * tonemod)) / 2;
    pwm_set_wrap(pwm_gpio_to_slice_num(pwm_pin), (125000000 / 125) / (tone * tonemod));
    saveSettings();
}

void loadSettings() {
    const settings* opts = (const settings*)(XIP_BASE + FLASH_TARGET_OFFSET);

    if (opts->speed == -1 || opts->tone == -1 || opts->tonemod == -1 || opts->keyerMode == -1) {
        speed = 15;
        tone = 440;
        tonemod = 1;
        keyerMode = 0;
        USBMode[0] = 1;
        USBMode[1] = 1;
        USBMode[2] = 1;
    } else {
        speed = opts->speed;
        tone = opts->tone;
        tonemod = opts->tonemod;
        keyerMode = opts->keyerMode;
        USBMode[0] = opts->USBMode[0];
        USBMode[1] = opts->USBMode[1];
        USBMode[2] = opts->USBMode[2];

    }
    saveSettings();
    setTone(tone);
    setSpeed(speed);
}

void saveSettings() {
    options = { speed, tone, tonemod, keyerMode, {USBMode[0],USBMode[1],USBMode[2]} };
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, i e s i  FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, (const uint8_t*)&options, FLASH_PAGE_SIZE);
    restore_interrupts(ints);
}