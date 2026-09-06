// LEDs.h
// Controle dos LEDs indicadores: Power (fixo), Mute, Clip (pulso ao
// detectar pico no DSP) e Signal (presença de sinal, opcional/heurístico).

#pragma once

#include <Arduino.h>
#include "ADAU1701.h"

class LedIndicators {
public:
    void begin();

    void setPower(bool on);
    void setMute(bool on);
    void setSignal(bool on);

    // Dispara o LED de clip por CLIP_LED_HOLD_MS.
    void pulseClip();

    // Consulta o(s) endereço(s) de detecção de nível do DSP (se
    // habilitado em DSPMap.h) e aciona pulseClip() automaticamente.
    // Chame periodicamente no loop().
    void pollClipDetect(ADAU1701 &dsp);

    // Atualiza o estado temporal do LED de clip (apaga após o hold time).
    void update();

private:
    unsigned long _clipOffAtMs = 0;
    bool _clipOn = false;
};

extern LedIndicators leds;
