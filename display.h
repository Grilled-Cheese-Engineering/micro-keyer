#pragma once
extern "C" {
    #include "ssd1306.h"
    #include "font.h"
}
#include "settings.h"
#include "hardware/watchdog.h"

extern ssd1306_t disp;
extern int speed;
extern int tone;

void drawMain();
