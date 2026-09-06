// ADAU1701.h
// Driver mínimo de controle do DSP ADAU1701 via I2C (control port),
// escrevendo diretamente em endereços de célula exportados do SigmaStudio.
//
// Formato de dados do ADAU1701: registradores de parâmetro (coeficientes,
// volumes, delays) usam ponto fixo 5.23 (1 bit de sinal + 4 inteiros +
// 23 fracionários = 28 bits), transmitidos em 4 bytes (MSB primeiro) com
// os 4 bits mais significativos como extensão de sinal.

#pragma once

#include <Arduino.h>
#include <stdint.h>

// Converte um double para o formato fixo 5.23 do ADAU1701 (int32_t com
// sinal, já pronto para separar em 4 bytes big-endian).
int32_t dspFloatToFixed(double value);

// Converte de volta (usado ao ler registradores de readback).
double dspFixedToFloat(int32_t fixed);

class ADAU1701 {
public:
    explicit ADAU1701(uint8_t i2cAddr);

    void begin();

    // Escreve 1 palavra de 28 bits (coeficiente, volume, etc) em um
    // endereço de célula.
    bool writeFixed(uint16_t cellAddr, double value);

    // Escreve N palavras consecutivas de 28 bits a partir de um endereço
    // (usado para os 5 coeficientes de um biquad: b0,b1,b2,-a1,-a2).
    bool writeFixedBlock(uint16_t cellAddr, const double *values, uint8_t count);

    // Escreve um valor bruto de 32 bits (já em ponto fixo) — usado por
    // volume/mute/delay quando o valor já foi calculado externamente.
    bool writeRaw32(uint16_t cellAddr, int32_t raw);

    // Mute: escreve 0.0 (mute) ou o ganho atual (unmute) na célula.
    bool setVolumeDb(uint16_t cellAddr, double db, bool muted);

    // Delay: valor em número de amostras (inteiro, célula de delay usa
    // formato de índice inteiro, não 5.23).
    bool setDelaySamples(uint16_t cellAddr, uint32_t samples);

    // Lê 4 bytes (readback) de um endereço, usado para o detector de clip.
    bool readRaw32(uint16_t cellAddr, int32_t &out);

private:
    uint8_t _addr;

    bool writeBytes(uint16_t cellAddr, const uint8_t *data, uint8_t len);
};
