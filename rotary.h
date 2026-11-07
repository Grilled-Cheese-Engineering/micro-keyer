#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include <functional>
#include <string>
#include <format>
#include <string_view>
#include <stdint.h>

int8_t table[] = {
    0,  1, -1,  0,
   -1,  0,  0,  1,
    1,  0,  0, -1,
    0, -1,  1,  0
};

class Encoder {
public:
    Encoder(int A, int B, std::function<void()> leftFunc, std::function<void()> rightFunc, gpio_irq_callback_t callback) : pinA(A), pinB(B), left(leftFunc), right(rightFunc) {
        gpio_init(A);
        gpio_set_dir(A, GPIO_IN);
        gpio_pull_up(A);

        gpio_init(B);
        gpio_set_dir(B, GPIO_IN);
        gpio_pull_up(B);

        int a = gpio_get(A);
        int b = gpio_get(B);
        if (!a && !b) {
            prevState = 3;
        } else if (a && !b) {
            prevState = 2;
        } else if (!a && b) {
            prevState = 1;
        }

        gpio_set_irq_enabled_with_callback(A, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, callback);
        gpio_set_irq_enabled_with_callback(B, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, callback);
        printf("created encoder A:%d, B%d\n", pinA, pinB);
    }



    void check(uint gpio, uint32_t events) {
        static uint8_t state = 0;
        static int8_t accumulator = 0;

        bool A = gpio_get(rot_b);
        bool B = gpio_get(rot_a);

        state = (state << 2) & 0x0F;
        state |= (A << 1) | B;

        int8_t movement = table[state];
        accumulator += movement;


        if (accumulator >= 4) {
            accumulator = 0;
            right();
        } else if (accumulator <= -4) {
            accumulator = 0;
            left();
        }
    }

    int pinA = -1;
    int pinB = -1;
    std::function<void()> left;
    std::function<void()> right;


    int prevState = 0;
};