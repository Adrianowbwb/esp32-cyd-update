#include "Encoder.h"

Encoder encoder;

int16_t Encoder::readSteps() {
    int16_t s = _steps;
    _steps = 0;
    return s;
}

bool Encoder::wasClicked() {
    bool c = _clicked;
    _clicked = false;
    return c;
}

bool Encoder::wasLongPressed() {
    bool l = _longPressed;
    _longPressed = false;
    return l;
}

void Encoder::simRotate(int16_t steps) {
    _steps = (int16_t)(_steps + steps);
}

void Encoder::simClick() {
    _clicked = true;
}

void Encoder::simLongPress() {
    _longPressed = true;
}
