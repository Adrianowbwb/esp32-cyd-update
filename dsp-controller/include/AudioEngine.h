// AudioEngine.h
// Camada que traduz a struct Settings (parâmetros "humanos": Hz, dB, ms)
// em escritas de registrador no ADAU1701, usando os endereços de célula
// definidos em DSPMap.h e os cálculos de biquad de Biquad.h.

#pragma once

#include "ADAU1701.h"
#include "Settings.h"

class AudioEngine {
public:
    explicit AudioEngine(ADAU1701 &dsp) : _dsp(dsp) {}

    // Aplica TODOS os parâmetros (usado no boot, após carregar da EEPROM).
    void applyAll(const Settings &s);

    // Aplicações parciais, chamadas pela UI a cada parâmetro editado —
    // evita recalcular/reenviar tudo a cada "tick" do encoder.
    void applyMasterVolumeMute(const Settings &s);
    void applyCrossover(const Settings &s);
    void applyEqBand(const Settings &s, uint8_t bandIndex);
    void applyTrims(const Settings &s);
    void applyDelay(const Settings &s);

private:
    ADAU1701 &_dsp;
};
