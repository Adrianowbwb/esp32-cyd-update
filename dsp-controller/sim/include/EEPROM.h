// EEPROM.h (shim para o simulador de terminal)
// Emula a EEPROM interna do ATmega, persistida em sim/sim_eeprom.bin
// para reproduzir de verdade o comportamento "restaura ao ligar".
#pragma once

#include <cstdint>
#include <cstring>
#include <cstdio>

class EEPROMClass {
public:
    EEPROMClass() { load(); }

    template <typename T>
    T &get(int address, T &t) {
        memcpy(&t, _data + address, sizeof(T));
        return t;
    }

    template <typename T>
    const T &put(int address, const T &t) {
        memcpy(_data + address, &t, sizeof(T));
        save();
        return t;
    }

private:
    static const size_t SIZE = 1024;
    uint8_t _data[SIZE] = {0};

    static const char *path() { return "sim_eeprom.bin"; }

    void load() {
        FILE *f = fopen(path(), "rb");
        if (f) {
            fread(_data, 1, SIZE, f);
            fclose(f);
        }
    }

    void save() {
        FILE *f = fopen(path(), "wb");
        if (f) {
            fwrite(_data, 1, SIZE, f);
            fclose(f);
        }
    }
};

extern EEPROMClass EEPROM;
