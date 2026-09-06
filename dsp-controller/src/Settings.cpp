#include "Settings.h"
#include "Config.h"
#include <EEPROM.h>
#include <string.h>
#include <stddef.h>

#define SETTINGS_MAGIC    0xA5
#define SETTINGS_VERSION  1
#define SETTINGS_EEPROM_ADDR 0

static uint8_t crc8(const uint8_t *data, size_t len) {
    uint8_t crc = 0x00;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t b = 0; b < 8; b++) {
            if (crc & 0x80) {
                crc = (uint8_t)((crc << 1) ^ 0x07);
            } else {
                crc = (uint8_t)(crc << 1);
            }
        }
    }
    return crc;
}

void settingsSetDefaults(Settings &s) {
    memset(&s, 0, sizeof(s));
    s.magic = SETTINGS_MAGIC;
    s.version = SETTINGS_VERSION;

    s.masterVolumeDb = -12.0f;
    s.masterMuted = false;

    s.xoverFreqHz = 2000.0f;

    // 3 bandas de EQ padrão: grave, médio, agudo — todas neutras (0dB)
    s.eq[0] = { 100.0f, 0.0f, 0.7f };
    s.eq[1] = { 1000.0f, 0.0f, 0.7f };
    s.eq[2] = { 8000.0f, 0.0f, 0.7f };

    s.gainLowDb = 0.0f;
    s.gainHighDb = 0.0f;

    s.delayLowSamples = 0;
    s.delayHighSamples = 0;
}

bool settingsLoad(Settings &s) {
    EEPROM.get(SETTINGS_EEPROM_ADDR, s);

    if (s.magic != SETTINGS_MAGIC || s.version != SETTINGS_VERSION) {
        settingsSetDefaults(s);
        return false;
    }

    uint8_t expected = crc8((const uint8_t *)&s, offsetof(Settings, crc));
    if (expected != s.crc) {
        settingsSetDefaults(s);
        return false;
    }
    return true;
}

void settingsSave(const Settings &s) {
    Settings toSave = s;
    toSave.crc = crc8((const uint8_t *)&toSave, offsetof(Settings, crc));
    EEPROM.put(SETTINGS_EEPROM_ADDR, toSave);
}

static bool s_saveScheduled = false;
static unsigned long s_saveAtMs = 0;

void settingsScheduleSave() {
    s_saveScheduled = true;
    s_saveAtMs = millis() + SAVE_DEBOUNCE_MS;
}

void settingsTick(const Settings &s) {
    if (s_saveScheduled && (long)(millis() - s_saveAtMs) >= 0) {
        s_saveScheduled = false;
        settingsSave(s);
    }
}
