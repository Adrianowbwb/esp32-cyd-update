// LiquidCrystal_I2C.h (shim para o simulador de terminal)
// Em vez de falar I2C com um backpack de verdade, mantém um buffer de
// texto 16x2 e redesenha no terminal a cada alteração.
#pragma once

#include <cstdint>

class LiquidCrystal_I2C {
public:
    LiquidCrystal_I2C(uint8_t addr, uint8_t cols, uint8_t rows);

    void init();
    void backlight();
    void noBacklight();
    void clear();
    void home();
    void setCursor(uint8_t col, uint8_t row);
    void print(const char *s);

private:
    uint8_t _cols;
    uint8_t _rows;
    char _buf[2][17];
    uint8_t _col = 0;
    uint8_t _row = 0;

    void flush();
};
