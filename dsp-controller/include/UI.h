// UI.h
// Menu no display 16x2 controlado inteiramente pelo encoder rotativo:
//   - Girar no HOME entra no menu.
//   - Girar no MENU navega entre os itens.
//   - Clique curto no MENU entra em modo de edição do item.
//   - Girar em EDIT altera o valor (aplicado ao DSP em tempo real).
//   - Clique curto em EDIT confirma e volta ao MENU.
//   - Pressão longa em EDIT cancela (restaura valor anterior) e volta ao MENU.
//   - Pressão longa no MENU volta ao HOME (salvando na EEPROM).

#pragma once

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "Settings.h"
#include "AudioEngine.h"

enum MenuItemId {
    ITEM_VOLUME = 0,
    ITEM_MUTE,
    ITEM_XOVER_FREQ,
    ITEM_EQ1_FREQ, ITEM_EQ1_GAIN, ITEM_EQ1_Q,
    ITEM_EQ2_FREQ, ITEM_EQ2_GAIN, ITEM_EQ2_Q,
    ITEM_EQ3_FREQ, ITEM_EQ3_GAIN, ITEM_EQ3_Q,
    ITEM_GAIN_LOW,
    ITEM_GAIN_HIGH,
    ITEM_DELAY_LOW,
    ITEM_DELAY_HIGH,
    ITEM_SAVE_EXIT,
    ITEM_COUNT
};

class UI {
public:
    UI(Settings &settings, AudioEngine &engine);

    void begin();
    void update();

private:
    enum State { STATE_HOME, STATE_MENU, STATE_EDIT };

    Settings &_s;
    AudioEngine &_engine;
    LiquidCrystal_I2C _lcd;

    State _state = STATE_HOME;
    int8_t _menuIndex = 0;
    unsigned long _lastActivityMs = 0;

    // snapshot do valor ao entrar em EDIT, para permitir cancelar
    float _editSnapshotF = 0;
    bool _editSnapshotB = false;
    uint16_t _editSnapshotU16 = 0;

    void goHome();
    void enterMenu();
    void enterEdit();
    void confirmEdit();
    void cancelEdit();

    void renderHome();
    void renderMenu();
    void renderEdit();

    void adjustItem(int16_t steps);
    void applyItem(uint8_t idx);
    void restoreSnapshot(uint8_t idx);

    const char *itemName(uint8_t idx);
    void formatItemValue(uint8_t idx, char *out, uint8_t outLen);

    // Auxiliares para os 3 campos (Freq/Gain/Q) das 3 bandas de EQ.
    bool isEqItem(uint8_t idx, uint8_t &band, uint8_t &field); // field: 0=freq,1=gain,2=q
};

extern UI *g_ui; // usado só para eventual acesso global, se necessário
