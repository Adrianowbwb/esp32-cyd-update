#!/usr/bin/env bash
# Compila o simulador de terminal do controlador DSP.
# Reaproveita a lógica real do firmware (Biquad, Settings, AudioEngine, UI)
# compilando nativamente para o PC, com hardware "falso" (sim/).
set -e

cd "$(dirname "$0")"

g++ -std=c++14 -O0 -g -Wall \
    -I include -I ../include \
    src/*.cpp \
    ../src/Biquad.cpp \
    ../src/Settings.cpp \
    ../src/AudioEngine.cpp \
    ../src/UI.cpp \
    -o dsp_sim

echo "OK: sim/dsp_sim gerado. Rode com: ./dsp_sim"
