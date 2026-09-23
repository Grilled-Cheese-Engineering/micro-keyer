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
        if (gpio == pinA || gpio == pinB) {
            int state = 0;
            int a = gpio_get(pinA);
            int b = gpio_get(pinB);
            if (!a && !b) {
                state = 3;
            } else if (a && !b) {
                state = 2;
            } else if (!a && b) {
                state = 1;
            }
            if (prevState == 0) {
                if (state == 2) {
                    right();
                } else if (state == 1) {
                    left();
                }
            } else if (prevState == 1) {
                if (state == 0) {
                    right();
                } else if (state == 3) {
                    left();
                }
            } else if (prevState == 2) {
                if (state == 3) {
                    right();
                } else if (state == 0) {
                    left();
                }
            } else if (prevState == 3) {
                if (state == 1) {
                    right();
                } else if (state == 2) {
                    left();
                }
            }
            prevState = state;
        }
    }
private:
    int pinA = -1;
    int pinB = -1;
    std::function<void()> left;
    std::function<void()> right;


    int prevState = 0;
};