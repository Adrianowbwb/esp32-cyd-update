#include "LiquidCrystal_I2C.h"
#include "SimState.h"
#include <cstdio>
#include <cstring>

LiquidCrystal_I2C::LiquidCrystal_I2C(uint8_t /*addr*/, uint8_t cols, uint8_t rows)
    : _cols(cols), _rows(rows) {
    clear();
}

void LiquidCrystal_I2C::init() {}
void LiquidCrystal_I2C::backlight() {}
void LiquidCrystal_I2C::noBacklight() {}

void LiquidCrystal_I2C::clear() {
    for (uint8_t r = 0; r < 2; r++) {
        memset(_buf[r], ' ', 16);
        _buf[r][16] = '\0';
    }
    _col = 0;
    _row = 0;
    flush();
}

void LiquidCrystal_I2C::home() {
    _col = 0;
    _row = 0;
}

void LiquidCrystal_I2C::setCursor(uint8_t col, uint8_t row) {
    _col = col;
    _row = row;
}

void LiquidCrystal_I2C::print(const char *s) {
    if (_row >= _rows) return;
    while (*s && _col < _cols) {
        _buf[_row][_col] = *s;
        _col++;
        s++;
    }
    flush();
}

void LiquidCrystal_I2C::flush() {
    printf("\033[2J\033[H");
    printf("+------------------+\n");
    printf("| %-16s |\n", _buf[0]);
    printf("| %-16s |\n", _buf[1]);
    printf("+------------------+\n");
    printf("\n");
    printf("Ultima escrita I2C -> DSP:\n");
    printf("  %s\n\n", g_lastDspWrite.c_str());
    printf("Controles: [a/d] girar 1 passo   [A/D] girar 3 passos\n");
    printf("           [espaco] clique       [l] pressao longa\n");
    printf("           [q] sair\n");
    fflush(stdout);
}
