// sim_main.cpp
// Simulador de terminal do controlador DSP: roda a MESMA lógica de
// menu/UI/AudioEngine/Settings do firmware real (src/UI.cpp,
// src/AudioEngine.cpp, src/Biquad.cpp, src/Settings.cpp), só trocando as
// camadas de hardware (LCD, encoder, I2C do DSP, EEPROM) por versões que
// rodam no PC. Não simula o processamento de áudio do ADAU1701 em si —
// só a lógica de controle (menu, cálculo de coeficientes, persistência).
//
// Controles: a/d = girar 1 passo | A/D = girar 3 passos
//            espaço = clique curto | l = pressão longa | q = sair

#include "Settings.h"
#include "AudioEngine.h"
#include "ADAU1701.h"
#include "UI.h"
#include "Encoder.h"
#include "DSPMap.h"

#include <cstdio>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>

static struct termios s_origTermios;

static void restoreTerminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &s_origTermios);
}

static void setRawTerminal() {
    tcgetattr(STDIN_FILENO, &s_origTermios);
    struct termios raw = s_origTermios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

int main() {
    setRawTerminal();

    Settings settings;
    settingsLoad(settings);

    ADAU1701 dsp(DSP_I2C_ADDR);
    AudioEngine engine(dsp);
    dsp.begin();
    engine.applyAll(settings);

    UI ui(settings, engine);
    ui.begin();

    bool running = true;
    while (running) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 150000; // sondagem periódica p/ permitir timeout de menu

        int ret = select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv);
        if (ret > 0 && FD_ISSET(STDIN_FILENO, &fds)) {
            char c = 0;
            if (read(STDIN_FILENO, &c, 1) == 1) {
                switch (c) {
                    case 'a': encoder.simRotate(-1); break;
                    case 'd': encoder.simRotate(1); break;
                    case 'A': encoder.simRotate(-3); break;
                    case 'D': encoder.simRotate(3); break;
                    case ' ':
                    case '\r':
                    case '\n':
                        encoder.simClick();
                        break;
                    case 'l': encoder.simLongPress(); break;
                    case 'q': running = false; break;
                    default: break;
                }
            }
        }

        encoder.update();
        ui.update();
        settingsTick(settings);
    }

    restoreTerminal();
    printf("\nSimulador encerrado. Configuracoes persistidas em sim_eeprom.bin\n");
    return 0;
}
