#include "UI.h"
#include "Config.h"
#include "DSPMap.h"
#include "Encoder.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

UI *g_ui = nullptr;

static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static uint16_t clampu16(int32_t v, uint16_t lo, uint16_t hi) {
    if (v < (int32_t)lo) return lo;
    if (v > (int32_t)hi) return hi;
    return (uint16_t)v;
}

// Passo "musical": multiplica/divide a frequência por 2^(1/12) por detente,
// dá uma sensação de ajuste proporcional (fino em graves, mais largo em
// agudos) muito mais usável num único encoder do que um passo linear fixo.
static float stepFrequency(float freqHz, int16_t steps) {
    float result = freqHz * powf(2.0f, steps / 12.0f);
    return clampf(result, 20.0f, 20000.0f);
}

UI::UI(Settings &settings, AudioEngine &engine)
    : _s(settings), _engine(engine), _lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS) {
    g_ui = this;
}

void UI::begin() {
    _lcd.init();
    _lcd.backlight();
    goHome();
}

void UI::goHome() {
    _state = STATE_HOME;
    _lastActivityMs = millis();
    renderHome();
}

void UI::enterMenu() {
    _state = STATE_MENU;
    _lastActivityMs = millis();
    renderMenu();
}

bool UI::isEqItem(uint8_t idx, uint8_t &band, uint8_t &field) {
    if (idx < ITEM_EQ1_FREQ || idx > ITEM_EQ3_Q) return false;
    uint8_t rel = idx - ITEM_EQ1_FREQ;
    band = rel / 3;
    field = rel % 3;
    return true;
}

const char *UI::itemName(uint8_t idx) {
    uint8_t band, field;
    if (isEqItem(idx, band, field)) {
        static const char *const freqNames[] = { "EQ1 Freq", "EQ2 Freq", "EQ3 Freq" };
        static const char *const gainNames[] = { "EQ1 Gain", "EQ2 Gain", "EQ3 Gain" };
        static const char *const qNames[]    = { "EQ1 Q",    "EQ2 Q",    "EQ3 Q" };
        return (field == 0) ? freqNames[band] : (field == 1) ? gainNames[band] : qNames[band];
    }
    switch (idx) {
        case ITEM_VOLUME:     return "Volume";
        case ITEM_MUTE:       return "Mute";
        case ITEM_XOVER_FREQ: return "Xover Freq";
        case ITEM_GAIN_LOW:   return "Trim Grave";
        case ITEM_GAIN_HIGH:  return "Trim Agudo";
        case ITEM_DELAY_LOW:  return "Delay Grave";
        case ITEM_DELAY_HIGH: return "Delay Agudo";
        case ITEM_SAVE_EXIT:  return "Salvar e sair";
        default:              return "?";
    }
}

void UI::formatItemValue(uint8_t idx, char *out, uint8_t outLen) {
    uint8_t band, field;
    char num[12];

    if (isEqItem(idx, band, field)) {
        EqBand b = _s.eq[band]; // cópia: campos de struct "packed" podem não estar alinhados
        if (field == 0) {
            snprintf(out, outLen, "%d Hz", (int)lroundf(b.freqHz));
        } else if (field == 1) {
            dtostrf(b.gainDb, 0, 1, num);
            snprintf(out, outLen, "%s dB", num);
        } else {
            dtostrf(b.q, 0, 2, num);
            snprintf(out, outLen, "Q=%s", num);
        }
        return;
    }

    switch (idx) {
        case ITEM_VOLUME:
            dtostrf(_s.masterVolumeDb, 0, 1, num);
            snprintf(out, outLen, "%s dB", num);
            break;
        case ITEM_MUTE:
            snprintf(out, outLen, "%s", _s.masterMuted ? "ON" : "OFF");
            break;
        case ITEM_XOVER_FREQ:
            snprintf(out, outLen, "%d Hz", (int)lroundf(_s.xoverFreqHz));
            break;
        case ITEM_GAIN_LOW:
            dtostrf(_s.gainLowDb, 0, 1, num);
            snprintf(out, outLen, "%s dB", num);
            break;
        case ITEM_GAIN_HIGH:
            dtostrf(_s.gainHighDb, 0, 1, num);
            snprintf(out, outLen, "%s dB", num);
            break;
        case ITEM_DELAY_LOW: {
            float ms = _s.delayLowSamples * 1000.0f / (float)DSP_SAMPLE_RATE_HZ;
            dtostrf(ms, 0, 2, num);
            snprintf(out, outLen, "%s ms", num);
            break;
        }
        case ITEM_DELAY_HIGH: {
            float ms = _s.delayHighSamples * 1000.0f / (float)DSP_SAMPLE_RATE_HZ;
            dtostrf(ms, 0, 2, num);
            snprintf(out, outLen, "%s ms", num);
            break;
        }
        case ITEM_SAVE_EXIT:
            out[0] = '\0';
            break;
        default:
            out[0] = '\0';
            break;
    }
}

void UI::renderHome() {
    char line[17];
    _lcd.clear();
    _lcd.setCursor(0, 0);
    _lcd.print("DSP ADAU1701");

    char volStr[8];
    dtostrf(_s.masterVolumeDb, 0, 1, volStr);
    snprintf(line, sizeof(line), "%sdB %s %dHz",
             volStr, _s.masterMuted ? "MUTE" : "    ", (int)lroundf(_s.xoverFreqHz));
    _lcd.setCursor(0, 1);
    _lcd.print(line);
}

void UI::renderMenu() {
    char valueStr[16];
    formatItemValue((uint8_t)_menuIndex, valueStr, sizeof(valueStr));

    _lcd.clear();
    _lcd.setCursor(0, 0);
    _lcd.print(">");
    _lcd.print(itemName((uint8_t)_menuIndex));

    _lcd.setCursor(0, 1);
    _lcd.print(valueStr);
}

void UI::renderEdit() {
    char valueStr[16];
    formatItemValue((uint8_t)_menuIndex, valueStr, sizeof(valueStr));

    _lcd.clear();
    _lcd.setCursor(0, 0);
    _lcd.print(itemName((uint8_t)_menuIndex));
    _lcd.print(" *");

    _lcd.setCursor(0, 1);
    _lcd.print("[");
    _lcd.print(valueStr);
    _lcd.print("]");
}

void UI::enterEdit() {
    if (_menuIndex == ITEM_SAVE_EXIT) {
        settingsSave(_s);
        _lcd.clear();
        _lcd.setCursor(0, 0);
        _lcd.print("Configuracoes");
        _lcd.setCursor(0, 1);
        _lcd.print("salvas!");
        delay(700);
        goHome();
        return;
    }

    uint8_t band, field;
    if (isEqItem((uint8_t)_menuIndex, band, field)) {
        if (field == 0) _editSnapshotF = _s.eq[band].freqHz;
        else if (field == 1) _editSnapshotF = _s.eq[band].gainDb;
        else _editSnapshotF = _s.eq[band].q;
    } else {
        switch (_menuIndex) {
            case ITEM_VOLUME:     _editSnapshotF = _s.masterVolumeDb; break;
            case ITEM_MUTE:       _editSnapshotB = _s.masterMuted; break;
            case ITEM_XOVER_FREQ: _editSnapshotF = _s.xoverFreqHz; break;
            case ITEM_GAIN_LOW:   _editSnapshotF = _s.gainLowDb; break;
            case ITEM_GAIN_HIGH:  _editSnapshotF = _s.gainHighDb; break;
            case ITEM_DELAY_LOW:  _editSnapshotU16 = _s.delayLowSamples; break;
            case ITEM_DELAY_HIGH: _editSnapshotU16 = _s.delayHighSamples; break;
            default: break;
        }
    }

    _state = STATE_EDIT;
    _lastActivityMs = millis();
    renderEdit();
}

void UI::restoreSnapshot(uint8_t idx) {
    uint8_t band, field;
    if (isEqItem(idx, band, field)) {
        if (field == 0) _s.eq[band].freqHz = _editSnapshotF;
        else if (field == 1) _s.eq[band].gainDb = _editSnapshotF;
        else _s.eq[band].q = _editSnapshotF;
        _engine.applyEqBand(_s, band);
        return;
    }

    switch (idx) {
        case ITEM_VOLUME:
            _s.masterVolumeDb = _editSnapshotF;
            _engine.applyMasterVolumeMute(_s);
            break;
        case ITEM_MUTE:
            _s.masterMuted = _editSnapshotB;
            _engine.applyMasterVolumeMute(_s);
            break;
        case ITEM_XOVER_FREQ:
            _s.xoverFreqHz = _editSnapshotF;
            _engine.applyCrossover(_s);
            break;
        case ITEM_GAIN_LOW:
            _s.gainLowDb = _editSnapshotF;
            _engine.applyTrims(_s);
            break;
        case ITEM_GAIN_HIGH:
            _s.gainHighDb = _editSnapshotF;
            _engine.applyTrims(_s);
            break;
        case ITEM_DELAY_LOW:
            _s.delayLowSamples = _editSnapshotU16;
            _engine.applyDelay(_s);
            break;
        case ITEM_DELAY_HIGH:
            _s.delayHighSamples = _editSnapshotU16;
            _engine.applyDelay(_s);
            break;
        default:
            break;
    }
}

void UI::confirmEdit() {
    _state = STATE_MENU;
    settingsScheduleSave();
    renderMenu();
}

void UI::cancelEdit() {
    restoreSnapshot((uint8_t)_menuIndex);
    _state = STATE_MENU;
    renderMenu();
}

void UI::applyItem(uint8_t idx) {
    uint8_t band, field;
    if (isEqItem(idx, band, field)) {
        _engine.applyEqBand(_s, band);
        return;
    }
    switch (idx) {
        case ITEM_VOLUME:
        case ITEM_MUTE:
            _engine.applyMasterVolumeMute(_s);
            break;
        case ITEM_XOVER_FREQ:
            _engine.applyCrossover(_s);
            break;
        case ITEM_GAIN_LOW:
        case ITEM_GAIN_HIGH:
            _engine.applyTrims(_s);
            break;
        case ITEM_DELAY_LOW:
        case ITEM_DELAY_HIGH:
            _engine.applyDelay(_s);
            break;
        default:
            break;
    }
}

void UI::adjustItem(int16_t steps) {
    if (steps == 0) return;

    uint8_t band, field;
    if (isEqItem((uint8_t)_menuIndex, band, field)) {
        // Copia local: `eq[]` fica dentro de uma struct "packed", então
        // seus campos podem não estar alinhados na memória — evitamos
        // criar uma referência direta a eles e regravamos o valor no final.
        EqBand b = _s.eq[band];
        if (field == 0) {
            b.freqHz = stepFrequency(b.freqHz, steps);
        } else if (field == 1) {
            b.gainDb = clampf(b.gainDb + steps * 0.5f, -12.0f, 12.0f);
        } else {
            b.q = clampf(b.q + steps * 0.05f, 0.30f, 10.0f);
        }
        _s.eq[band] = b;
        applyItem((uint8_t)_menuIndex);
        return;
    }

    switch (_menuIndex) {
        case ITEM_VOLUME:
            _s.masterVolumeDb = clampf(_s.masterVolumeDb + steps * 0.5f, -60.0f, 0.0f);
            break;
        case ITEM_MUTE:
            if (steps != 0) _s.masterMuted = !_s.masterMuted;
            break;
        case ITEM_XOVER_FREQ:
            _s.xoverFreqHz = stepFrequency(_s.xoverFreqHz, steps);
            break;
        case ITEM_GAIN_LOW:
            _s.gainLowDb = clampf(_s.gainLowDb + steps * 0.5f, -12.0f, 12.0f);
            break;
        case ITEM_GAIN_HIGH:
            _s.gainHighDb = clampf(_s.gainHighDb + steps * 0.5f, -12.0f, 12.0f);
            break;
        case ITEM_DELAY_LOW: {
            int32_t v = (int32_t)_s.delayLowSamples + steps * 4;
            _s.delayLowSamples = clampu16(v, 0, DSP_MAX_DELAY_SAMPLES);
            break;
        }
        case ITEM_DELAY_HIGH: {
            int32_t v = (int32_t)_s.delayHighSamples + steps * 4;
            _s.delayHighSamples = clampu16(v, 0, DSP_MAX_DELAY_SAMPLES);
            break;
        }
        default:
            break;
    }
    applyItem((uint8_t)_menuIndex);
}

void UI::update() {
    int16_t steps = encoder.readSteps();
    bool clicked = encoder.wasClicked();
    bool longPressed = encoder.wasLongPressed();
    unsigned long now = millis();

    switch (_state) {
        case STATE_HOME:
            if (steps != 0 || clicked) {
                enterMenu();
            } else {
                // Atualiza a tela home periodicamente para refletir mudanças
                // feitas por outra via (ex.: nenhuma neste projeto, mas
                // mantém a home "viva").
            }
            break;

        case STATE_MENU:
            if (steps != 0) {
                int16_t idx = (int16_t)_menuIndex + steps;
                idx %= ITEM_COUNT;
                if (idx < 0) idx += ITEM_COUNT;
                _menuIndex = (int8_t)idx;
                _lastActivityMs = now;
                renderMenu();
            }
            if (clicked) {
                enterEdit();
            }
            if (longPressed) {
                settingsScheduleSave();
                goHome();
            }
            if ((now - _lastActivityMs) >= MENU_IDLE_TIMEOUT_MS) {
                settingsScheduleSave();
                goHome();
            }
            break;

        case STATE_EDIT:
            if (steps != 0) {
                adjustItem(steps);
                _lastActivityMs = now;
                renderEdit();
            }
            if (clicked) {
                confirmEdit();
            }
            if (longPressed) {
                cancelEdit();
            }
            break;
    }
}
