// Encoder.h (shim para o simulador de terminal)
// Mesma API pública usada por UI.cpp/main.cpp (begin/update/readSteps/
// wasClicked/wasLongPressed), mas os eventos vêm do teclado em vez de
// uma ISR de quadratura.
#pragma once

#include <cstdint>

class Encoder {
public:
    void begin() {}
    void update() {}

    int16_t readSteps();
    bool wasClicked();
    bool wasLongPressed();

    // Usado só pelo sim_main.cpp para injetar eventos de teclado.
    void simRotate(int16_t steps);
    void simClick();
    void simLongPress();

private:
    int16_t _steps = 0;
    bool _clicked = false;
    bool _longPressed = false;
};

extern Encoder encoder;
