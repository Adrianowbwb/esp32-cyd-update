// Controlador DSP para ADAU1701 baseado em ATmega328PB
// Encoder rotativo + LCD 16x2 + LEDs indicadores
// Crossover (Linkwitz-Riley 24dB/oitava) / EQ paramétrico 3 bandas /
// Delay por via / Trim de ganho por via / Volume+Mute master
//
// Ver README.md para fiação e para o schematic SigmaStudio esperado.

#include <Arduino.h>
#include "Config.h"
#include "DSPMap.h"
#include "ADAU1701.h"
#include "AudioEngine.h"
#include "Settings.h"
#include "Encoder.h"
#include "LEDs.h"
#include "UI.h"

static Settings g_settings;
static ADAU1701 g_dsp(DSP_I2C_ADDR);
static AudioEngine g_engine(g_dsp);
static UI g_uiInstance(g_settings, g_engine);

static unsigned long s_lastClipPollMs = 0;
#define CLIP_POLL_INTERVAL_MS 50

void setup() {
    bool loadedOk = settingsLoad(g_settings);

    encoder.begin();
    leds.begin();
    g_dsp.begin();
    g_uiInstance.begin();

    // Aguarda o ADAU1701 terminar o auto-boot (carregamento do programa a
    // partir da EEPROM externa) antes de sobrescrever parâmetros via I2C.
    delay(DSP_BOOT_DELAY_MS);

    // Aplica tudo que veio da EEPROM (ou os defaults, se a EEPROM estava
    // vazia/corrompida) direto no DSP assim que o sistema liga.
    g_engine.applyAll(g_settings);

    leds.setPower(true);
    leds.setMute(g_settings.masterMuted);

    if (!loadedOk) {
        settingsScheduleSave();
    }
}

void loop() {
    encoder.update();
    g_uiInstance.update();

    leds.setMute(g_settings.masterMuted);
    leds.update();

    unsigned long now = millis();
    if ((now - s_lastClipPollMs) >= CLIP_POLL_INTERVAL_MS) {
        s_lastClipPollMs = now;
        leds.pollClipDetect(g_dsp);
    }

    settingsTick(g_settings);
}
