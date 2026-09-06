#include "Encoder.h"
#include "Config.h"

Encoder encoder;

// Tabela de transição de quadratura de 4 bits: (estado anterior AB)<<2 | (estado atual AB)
// Retorna +1, -1 ou 0 por transição válida/​inválida. Acumula em quartos de detente.
static const int8_t QUAD_TABLE[16] = {
    0, -1,  1,  0,
    1,  0,  0, -1,
   -1,  0,  0,  1,
    0,  1, -1,  0
};

static volatile uint8_t s_quadState = 0;

static void encoderISR() {
    uint8_t a = digitalRead(PIN_ENC_A);
    uint8_t b = digitalRead(PIN_ENC_B);
    uint8_t curr = (uint8_t)((a << 1) | b);
    s_quadState = (uint8_t)(((s_quadState << 2) | curr) & 0x0F);
    encoder.isrHandler();
}

void Encoder::isrHandler() {
    _rawDelta = (int16_t)(_rawDelta + QUAD_TABLE[s_quadState]);
}

void Encoder::begin() {
    pinMode(PIN_ENC_A, INPUT_PULLUP);
    pinMode(PIN_ENC_B, INPUT_PULLUP);
    pinMode(PIN_ENC_SW, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(PIN_ENC_A), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ENC_B), encoderISR, CHANGE);

    _btnRawLast = digitalRead(PIN_ENC_SW) == LOW;
    _btnState = _btnRawLast;
    _btnChangeMs = millis();
}

int16_t Encoder::readSteps() {
    int16_t quarters;
    noInterrupts();
    quarters = _rawDelta;
    _rawDelta = 0;
    interrupts();

    // 4 transições de quadratura = 1 detente físico na maioria dos
    // encoders KY-040/EC11.
    _stepAccumulator = (int16_t)(_stepAccumulator + quarters);
    int16_t steps = (int16_t)(_stepAccumulator / 4);
    _stepAccumulator = (int16_t)(_stepAccumulator % 4);
    return steps;
}

void Encoder::update() {
    bool rawPressed = digitalRead(PIN_ENC_SW) == LOW;
    unsigned long now = millis();

    if (rawPressed != _btnRawLast) {
        _btnRawLast = rawPressed;
        _btnChangeMs = now;
    }

    if ((now - _btnChangeMs) >= ENCODER_DEBOUNCE_MS && rawPressed != _btnState) {
        _btnState = rawPressed;
        if (_btnState) {
            // borda de descida (pressionou)
            _btnPressStartMs = now;
            _longPressFired = false;
        } else {
            // borda de subida (soltou)
            if (!_longPressFired) {
                _clickPending = true;
            }
        }
    }

    if (_btnState && !_longPressFired &&
        (now - _btnPressStartMs) >= ENCODER_LONGPRESS_MS) {
        _longPressFired = true;
        _longPressPending = true;
    }
}

bool Encoder::wasClicked() {
    if (_clickPending) {
        _clickPending = false;
        return true;
    }
    return false;
}

bool Encoder::wasLongPressed() {
    if (_longPressPending) {
        _longPressPending = false;
        return true;
    }
    return false;
}
