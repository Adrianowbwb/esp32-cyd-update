// Biquad.h
// Cálculo de coeficientes de biquad (fórmulas "RBJ cookbook") no formato
// esperado pela célula "General 2nd Order" do SigmaStudio:
//   saída[0..4] = { b0, b1, b2, -a1, -a2 }  (já normalizados por a0)
// onde a equação de diferenças é:
//   y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] + (-a1)*y[n-1] + (-a2)*y[n-2]

#pragma once

#include <stdint.h>

// Q de Butterworth para usar em cascata de 2 estágios = filtro
// Linkwitz-Riley de 4ª ordem (24 dB/oitava).
#define BUTTERWORTH_Q  0.70710678

// Preenche coeffs[5] com um lowpass de 2ª ordem (Direct Form I, RBJ).
void biquadLowpass(double freqHz, double q, double sampleRateHz, double coeffs[5]);

// Preenche coeffs[5] com um highpass de 2ª ordem (Direct Form I, RBJ).
void biquadHighpass(double freqHz, double q, double sampleRateHz, double coeffs[5]);

// Preenche coeffs[5] com um filtro paramétrico "peaking EQ" (RBJ).
void biquadPeakingEQ(double freqHz, double q, double gainDb, double sampleRateHz, double coeffs[5]);

// Filtro "passa-tudo" (bypass), útil para desativar uma banda de EQ.
void biquadBypass(double coeffs[5]);
