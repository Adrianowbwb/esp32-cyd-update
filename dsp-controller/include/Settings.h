// Settings.h
// Todos os parâmetros de áudio editáveis pelo usuário, persistidos na
// EEPROM interna do ATmega328PB. Os controles de EQ e crossover são
// "linkados" (mesmo valor aplicado a L e R); os trims de ganho e delay
// por via existem em uma única cópia também linkada, mas as escritas no
// DSP endereçam os dois canais separadamente (ver DSPMap.h) para permitir
// evoluir para controle independente por canal no futuro.

#pragma once

#include <stdint.h>
#include "DSPMap.h"

struct EqBand {
    float freqHz;
    float gainDb;
    float q;
};

struct Settings {
    uint8_t magic;      // marcador de struct válida
    uint8_t version;    // versão do layout, para invalidar EEPROM antiga

    float masterVolumeDb;   // -60.0 .. 0.0
    bool  masterMuted;

    float xoverFreqHz;       // 40 .. 20000 (Linkwitz-Riley 24dB/oitava)

    EqBand eq[EQ_NUM_BANDS]; // EQ paramétrico pré-crossover

    float gainLowDb;         // trim da via grave, -12..+12
    float gainHighDb;        // trim da via aguda, -12..+12

    uint16_t delayLowSamples;   // 0 .. DSP_MAX_DELAY_SAMPLES
    uint16_t delayHighSamples;  // 0 .. DSP_MAX_DELAY_SAMPLES

    uint8_t crc; // CRC8 dos campos acima (calculado/verificado por load/save)
};

// Preenche `s` com valores padrão de fábrica.
void settingsSetDefaults(Settings &s);

// Carrega da EEPROM. Se o conteúdo for inválido (CRC/magic/versão não
// batem), preenche com defaults e retorna false.
bool settingsLoad(Settings &s);

// Grava imediatamente na EEPROM (só regrava bytes que mudaram).
void settingsSave(const Settings &s);

// Agenda uma gravação para daqui a SAVE_DEBOUNCE_MS (evita desgastar a
// EEPROM a cada "tick" do encoder). Chame settingsTick() no loop().
void settingsScheduleSave();
void settingsTick(const Settings &s);
