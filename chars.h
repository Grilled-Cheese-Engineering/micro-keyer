#include <vector>
#include <string>
const uint dit = 1;
const uint dah = 0;
const uint gap = 2;
const uint space = 3;
const uint end = 4;

int dit_pin = 26;
int dah_pin = 27;
int pwm_pin = 28;

int uart_rx_pin = 1;
int uart_tx_pin = 0;

int rot_a = 5;
int rot_b = 2;
int rot_sw = 3;

int display_scl = 7;
int display_sda = 6;
#define I2C_PORT i2c1


std::string sp = " ";
std::vector<std::vector<int>> chars = {
    {dit, dah},                     // A
    {dah, dit, dit, dit},           // B
    {dah, dit, dah, dit},           // C
    {dah, dit, dit},                // D
    {dit},                          // E
    {dit, dit, dah, dit},           // F
    {dah, dah, dit},                // G
    {dit, dit, dit, dit},           // H
    {dit, dit},                     // I
    {dit, dah, dah, dah},           // J
    {dah, dit, dah},                // K
    {dit, dah, dit, dit},           // L
    {dah, dah},                     // M
    {dah, dit},                     // N
    {dah, dah, dah},                // O
    {dit, dah, dah, dit},           // P
    {dah, dah, dit, dah},           // Q
    {dit, dah, dit},                // R
    {dit, dit, dit},                // S
    {dah},                          // T
    {dit, dit, dah},                // U
    {dit, dit, dit, dah},           // V
    {dit, dah, dah},                // W
    {dah, dit, dit, dah},           // X
    {dah, dit, dah, dah},           // Y
    {dah, dah, dit, dit},           // Z
    {dit, dah, dah, dah, dah},      // 1
    {dit, dit, dah, dah, dah},      // 2
    {dit, dit, dit, dah, dah},      // 3
    {dit, dit, dit, dit, dah},      // 4
    {dit, dit, dit, dit, dit},      // 5
    {dah, dit, dit, dit, dit},      // 6
    {dah, dah, dit, dit, dit},      // 7
    {dah, dah, dah, dit, dit},      // 8
    {dah, dah, dah, dah, dit},      // 9
    {dah, dah, dah, dah, dah},      // 0
    {dit, dit, dah, dah, dit, dit}, //?
    {dah, dah, dit, dit, dah, dah}, //,
    {dit, dah, dit, dah, dit, dah}, //.
    {dah, dit, dit, dah, dit},      //"/"
    {dah, dit, dit, dit, dah, dit, dah} // <BK>

};

std::vector<std::string> strs = {
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "0",
    "?",
    ",",
    ".",
    "/",
    "<BK>"
};