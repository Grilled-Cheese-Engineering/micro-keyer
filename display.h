#ifndef MENU_OPTION_H
#define MENU_OPTION_H
extern "C" {
    #include "ssd1306.h"
    #include "font.h"
}
#include "hardware/watchdog.h"
#include <string>
#include <vector>
#include <functional>
#include "hardware/flash.h"
#include "hardware/sync.h"

extern ssd1306_t disp;

extern int speed;
extern int tone;
extern int tonemod;
extern bool USBMode[3];
extern int keyerMode;

extern int selected_item;
extern int clicked_item;



extern std::string decode;

void setSpeed(int x);
void setTone(int x);

void drawMain();
void drawMenu();

void loadSettings();
void saveSettings();


#define MODE_CDC_SERIAL  0
#define MODE_HID 1
#define MODE_MIDI 2

#define FLASH_TARGET_OFFSET (2 * 1024 * 1024 - FLASH_SECTOR_SIZE)


typedef struct __attribute__((aligned(FLASH_PAGE_SIZE))) {
    int speed;
    int tone;
    int tonemod;
    int keyerMode;
    bool USBMode[3];
} settings;

extern settings options;

class MenuOption {
public:

    MenuOption(std::string nameStr, std::string format, int val, int maxVal, int minVal, std::function<void(MenuOption&)> inc, std::function<void(MenuOption&)> dec, std::function<void(MenuOption&)> app, std::function<void(MenuOption&)> load) :
        name(nameStr), format(format), value(val), incVal(inc), decVal(dec), appVal(app), loadVal(load), maxValue(maxVal), minValue(minVal), useBool(false) {
    }

    MenuOption(std::string nameStr, std::vector<std::string> strArr, int val, std::function<void(MenuOption&)> inc, std::function<void(MenuOption&)> dec, std::function<void(MenuOption&)> app, std::function<void(MenuOption&)> load) :
        name(nameStr), arr(strArr), incVal(inc), decVal(dec), appVal(app), loadVal(load), useValueIndex(true), useBool(false) {
        if (val >= 0 && val < arr.size()) {
            valueIndex = val;
        }
    }

    MenuOption(std::string nameStr, int val, std::function<void(MenuOption&)> inc, std::function<void(MenuOption&)> dec, std::function<void(MenuOption&)> app, std::function<void(MenuOption&)> load) :
        name(nameStr), incVal(inc), decVal(dec), appVal(app), loadVal(load), useValueIndex(false), useBool(true) {
        if (val >= 0 && val < arr.size()) {
            valueIndex = val;
        }
    }

    std::string name = "Option";
    std::string format = "{}";
    bool useValueIndex = false;
    bool useBool = false;
    uint32_t valueIndex = 0;
    std::vector<std::string> arr;
    int value;
    int maxValue;
    int minValue = 0;
    std::function<void(MenuOption&)> incVal;
    std::function<void(MenuOption&)> decVal;
    std::function<void(MenuOption&)> appVal;
    std::function<void(MenuOption&)> loadVal;
    void turnL() {
        decVal(*this);
    }
    void turnR() {
        incVal(*this);
    }
    void click() {
        appVal(*this);
    }
    void load() {
        loadVal(*this);
    }
};


extern std::vector<MenuOption> optionList;

#endif