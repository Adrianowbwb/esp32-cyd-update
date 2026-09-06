#include "ADAU1701.h"
#include <Wire.h>

// 1.0 em 5.23 fixo = 2^23
static const double FIXED_5_23_SCALE = 8388608.0; // 2^23
static const int32_t FIXED_MIN = -134217728L;      // -2^27
static const int32_t FIXED_MAX = 134217727L;       //  2^27 - 1

int32_t dspFloatToFixed(double value) {
    double scaled = value * FIXED_5_23_SCALE;
    if (scaled > (double)FIXED_MAX) scaled = (double)FIXED_MAX;
    if (scaled < (double)FIXED_MIN) scaled = (double)FIXED_MIN;
    long r = lround(scaled);
    return (int32_t)r;
}

double dspFixedToFloat(int32_t fixed) {
    return (double)fixed / FIXED_5_23_SCALE;
}

ADAU1701::ADAU1701(uint8_t i2cAddr) : _addr(i2cAddr) {}

void ADAU1701::begin() {
    Wire.begin();
    Wire.setClock(100000UL);
}

bool ADAU1701::writeBytes(uint16_t cellAddr, const uint8_t *data, uint8_t len) {
    Wire.beginTransmission(_addr);
    Wire.write((uint8_t)(cellAddr >> 8));
    Wire.write((uint8_t)(cellAddr & 0xFF));
    for (uint8_t i = 0; i < len; i++) {
        Wire.write(data[i]);
    }
    uint8_t result = Wire.endTransmission();
    return result == 0;
}

bool ADAU1701::writeRaw32(uint16_t cellAddr, int32_t raw) {
    uint8_t buf[4];
    // Os 28 bits úteis ficam nos 28 bits menos significativos; os 4 bits
    // superiores do primeiro byte carregam a extensão de sinal, exatamente
    // como o ADAU1701 espera (registrador de 28 bits em 4 bytes MSB-first).
    buf[0] = (uint8_t)((raw >> 24) & 0xFF);
    buf[1] = (uint8_t)((raw >> 16) & 0xFF);
    buf[2] = (uint8_t)((raw >> 8) & 0xFF);
    buf[3] = (uint8_t)(raw & 0xFF);
    return writeBytes(cellAddr, buf, 4);
}

bool ADAU1701::writeFixed(uint16_t cellAddr, double value) {
    return writeRaw32(cellAddr, dspFloatToFixed(value));
}

bool ADAU1701::writeFixedBlock(uint16_t cellAddr, const double *values, uint8_t count) {
    // Cada palavra ocupa um endereço de célula sequencial (b0, b1, b2,
    // -a1, -a2 no caso de um biquad "General 2nd order").
    bool ok = true;
    for (uint8_t i = 0; i < count; i++) {
        ok &= writeFixed((uint16_t)(cellAddr + i), values[i]);
    }
    return ok;
}

bool ADAU1701::setVolumeDb(uint16_t cellAddr, double db, bool muted) {
    if (muted) {
        return writeFixed(cellAddr, 0.0);
    }
    // dB -> linear (célula de volume no SigmaStudio espera ganho linear,
    // não dB, em 5.23).
    double linear = pow(10.0, db / 20.0);
    return writeFixed(cellAddr, linear);
}

bool ADAU1701::setDelaySamples(uint16_t cellAddr, uint32_t samples) {
    // A célula de delay do SigmaStudio usa um inteiro (número de amostras),
    // não 5.23 — escrevemos o valor bruto de 32 bits.
    return writeRaw32(cellAddr, (int32_t)samples);
}

bool ADAU1701::readRaw32(uint16_t cellAddr, int32_t &out) {
    Wire.beginTransmission(_addr);
    Wire.write((uint8_t)(cellAddr >> 8));
    Wire.write((uint8_t)(cellAddr & 0xFF));
    if (Wire.endTransmission(false) != 0) {
        return false;
    }
    uint8_t n = Wire.requestFrom((int)_addr, 4, (int)true);
    if (n != 4) {
        return false;
    }
    uint8_t b0 = Wire.read();
    uint8_t b1 = Wire.read();
    uint8_t b2 = Wire.read();
    uint8_t b3 = Wire.read();
    int32_t raw = ((int32_t)b0 << 24) | ((int32_t)b1 << 16) | ((int32_t)b2 << 8) | b3;
    out = raw;
    return true;
}
