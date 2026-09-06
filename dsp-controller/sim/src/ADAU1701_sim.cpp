// ADAU1701_sim.cpp
// Implementação alternativa de ADAU1701.h (mesma interface de
// include/ADAU1701.h) para o simulador: em vez de escrever via I2C no
// chip de verdade, registra em texto o que seria enviado. Isso permite
// validar a lógica da UI e os cálculos de biquad/volume/delay sem
// hardware — a resposta de áudio real continua exigindo o chip (ou o
// próprio simulador do SigmaStudio).

#include "ADAU1701.h"
#include "SimState.h"
#include <cstdio>

std::string g_lastDspWrite = "(nenhuma ainda)";

ADAU1701::ADAU1701(uint8_t i2cAddr) : _addr(i2cAddr) {}

void ADAU1701::begin() {}

bool ADAU1701::writeFixed(uint16_t cellAddr, double value) {
    char buf[96];
    snprintf(buf, sizeof(buf), "0x%04X <- %.6f", cellAddr, value);
    g_lastDspWrite = buf;
    return true;
}

bool ADAU1701::writeFixedBlock(uint16_t cellAddr, const double *values, uint8_t count) {
    char buf[160];
    int n = snprintf(buf, sizeof(buf), "0x%04X biquad:", cellAddr);
    static const char *labels[5] = { "b0", "b1", "b2", "-a1", "-a2" };
    for (uint8_t i = 0; i < count && i < 5 && n < (int)sizeof(buf); i++) {
        n += snprintf(buf + n, sizeof(buf) - n, " %s=%.4f", labels[i], values[i]);
    }
    g_lastDspWrite = buf;
    return true;
}

bool ADAU1701::writeRaw32(uint16_t cellAddr, int32_t raw) {
    char buf[64];
    snprintf(buf, sizeof(buf), "0x%04X <- raw 0x%08X", cellAddr, (unsigned)raw);
    g_lastDspWrite = buf;
    return true;
}

bool ADAU1701::setVolumeDb(uint16_t cellAddr, double db, bool muted) {
    char buf[80];
    if (muted) {
        snprintf(buf, sizeof(buf), "0x%04X <- MUTE (ganho 0.0)", cellAddr);
    } else {
        snprintf(buf, sizeof(buf), "0x%04X <- volume %.1f dB", cellAddr, db);
    }
    g_lastDspWrite = buf;
    return true;
}

bool ADAU1701::setDelaySamples(uint16_t cellAddr, uint32_t samples) {
    char buf[80];
    snprintf(buf, sizeof(buf), "0x%04X <- delay %u amostras", cellAddr, (unsigned)samples);
    g_lastDspWrite = buf;
    return true;
}

bool ADAU1701::readRaw32(uint16_t /*cellAddr*/, int32_t &out) {
    out = 0;
    return true;
}
