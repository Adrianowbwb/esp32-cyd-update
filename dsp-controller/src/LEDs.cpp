#include "LEDs.h"
#include "Config.h"
#include "DSPMap.h"

LedIndicators leds;

void LedIndicators::begin() {
    pinMode(PIN_LED_POWER, OUTPUT);
    pinMode(PIN_LED_MUTE, OUTPUT);
    pinMode(PIN_LED_CLIP, OUTPUT);
    pinMode(PIN_LED_SIGNAL, OUTPUT);

    digitalWrite(PIN_LED_POWER, LOW);
    digitalWrite(PIN_LED_MUTE, LOW);
    digitalWrite(PIN_LED_CLIP, LOW);
    digitalWrite(PIN_LED_SIGNAL, LOW);
}

void LedIndicators::setPower(bool on) {
    digitalWrite(PIN_LED_POWER, on ? HIGH : LOW);
}

void LedIndicators::setMute(bool on) {
    digitalWrite(PIN_LED_MUTE, on ? HIGH : LOW);
}

void LedIndicators::setSignal(bool on) {
    digitalWrite(PIN_LED_SIGNAL, on ? HIGH : LOW);
}

void LedIndicators::pulseClip() {
    _clipOn = true;
    digitalWrite(PIN_LED_CLIP, HIGH);
    _clipOffAtMs = millis() + CLIP_LED_HOLD_MS;
}

void LedIndicators::update() {
    if (_clipOn && (long)(millis() - _clipOffAtMs) >= 0) {
        _clipOn = false;
        digitalWrite(PIN_LED_CLIP, LOW);
    }
}

void LedIndicators::pollClipDetect(ADAU1701 &dsp) {
#if DSP_HAS_CLIP_DETECT
    for (uint8_t ch = 0; ch < NUM_CH; ch++) {
        int32_t raw = 0;
        if (dsp.readRaw32(CLIP_DETECT_ADDR[ch], raw)) {
            if ((uint32_t)raw >= DSP_CLIP_THRESHOLD_FIXED) {
                pulseClip();
            }
        }
    }
#else
    (void)dsp;
#endif
}
