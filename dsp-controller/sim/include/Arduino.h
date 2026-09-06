// Arduino.h (shim para o simulador de terminal)
// Fornece só o suficiente para compilar include/*.h e src/*.cpp do
// projeto real fora do AVR, sem tocar hardware nenhum.
#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <chrono>
#include <thread>

typedef uint8_t byte;

#define HIGH 1
#define LOW  0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define CHANGE 1
#define RISING 2
#define FALLING 3

inline unsigned long millis() {
    using namespace std::chrono;
    static const auto t0 = steady_clock::now();
    return (unsigned long)duration_cast<milliseconds>(steady_clock::now() - t0).count();
}

inline void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline void pinMode(uint8_t, uint8_t) {}
inline void digitalWrite(uint8_t, uint8_t) {}
inline int digitalRead(uint8_t) { return LOW; }

inline void noInterrupts() {}
inline void interrupts() {}
inline int digitalPinToInterrupt(int p) { return p; }
inline void attachInterrupt(int, void (*)(), int) {}

#define F(x) (x)

// AVR-libc oferece dtostrf() (usado em UI.cpp para formatar floats sem
// puxar a libc de ponto flutuante do printf); a libc do host não tem
// essa função, então replicamos a mesma assinatura aqui.
inline char *dtostrf(double val, signed char width, unsigned char prec, char *s) {
    char fmt[16];
    snprintf(fmt, sizeof(fmt), "%%%d.%df", width, prec);
    snprintf(s, 32, fmt, val);
    return s;
}
