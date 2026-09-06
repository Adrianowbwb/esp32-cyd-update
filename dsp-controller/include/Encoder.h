// Encoder.h
// Leitura de encoder rotativo incremental (tipo KY-040) por interrupção,
// com decodificação de quadratura por tabela de transição (robusta contra
// ruído de contato) e leitura do botão com debounce + detecção de
// clique curto / pressão longa.

#pragma once

#include <Arduino.h>

class Encoder {
public:
    void begin();

    // Deve ser chamado a cada iteração do loop() para tratar o botão.
    void update();

    // Retorna quantos "detentes" o encoder girou desde a última chamada
    // (positivo = sentido horário, negativo = anti-horário) e zera o
    // acumulador.
    int16_t readSteps();

    // Verdadeiro por um único update() após um clique curto (aperta+solta
    // antes de ENCODER_LONGPRESS_MS).
    bool wasClicked();

    // Verdadeiro por um único update() assim que o botão atinge o tempo
    // de pressão longa (não espera soltar).
    bool wasLongPressed();

    // Chamado pela ISR — não usar diretamente.
    void isrHandler();

private:
    volatile int16_t _rawDelta = 0; // em quartos de detente
    int16_t _stepAccumulator = 0;

    bool _btnState = false;       // true = pressionado (nível já com debounce)
    bool _btnRawLast = false;
    unsigned long _btnChangeMs = 0;
    unsigned long _btnPressStartMs = 0;
    bool _longPressFired = false;
    bool _clickPending = false;
    bool _longPressPending = false;
};

extern Encoder encoder;
