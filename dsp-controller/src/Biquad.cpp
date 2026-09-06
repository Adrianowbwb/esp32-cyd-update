#include "Biquad.h"
#include <math.h>

static void storeNormalized(double b0, double b1, double b2,
                             double a0, double a1, double a2,
                             double coeffs[5]) {
    coeffs[0] = b0 / a0;
    coeffs[1] = b1 / a0;
    coeffs[2] = b2 / a0;
    coeffs[3] = -a1 / a0;
    coeffs[4] = -a2 / a0;
}

void biquadLowpass(double freqHz, double q, double sampleRateHz, double coeffs[5]) {
    double w0 = 2.0 * M_PI * freqHz / sampleRateHz;
    double cosw0 = cos(w0);
    double sinw0 = sin(w0);
    double alpha = sinw0 / (2.0 * q);

    double b0 = (1.0 - cosw0) / 2.0;
    double b1 = 1.0 - cosw0;
    double b2 = (1.0 - cosw0) / 2.0;
    double a0 = 1.0 + alpha;
    double a1 = -2.0 * cosw0;
    double a2 = 1.0 - alpha;

    storeNormalized(b0, b1, b2, a0, a1, a2, coeffs);
}

void biquadHighpass(double freqHz, double q, double sampleRateHz, double coeffs[5]) {
    double w0 = 2.0 * M_PI * freqHz / sampleRateHz;
    double cosw0 = cos(w0);
    double sinw0 = sin(w0);
    double alpha = sinw0 / (2.0 * q);

    double b0 = (1.0 + cosw0) / 2.0;
    double b1 = -(1.0 + cosw0);
    double b2 = (1.0 + cosw0) / 2.0;
    double a0 = 1.0 + alpha;
    double a1 = -2.0 * cosw0;
    double a2 = 1.0 - alpha;

    storeNormalized(b0, b1, b2, a0, a1, a2, coeffs);
}

void biquadPeakingEQ(double freqHz, double q, double gainDb, double sampleRateHz, double coeffs[5]) {
    double A = pow(10.0, gainDb / 40.0);
    double w0 = 2.0 * M_PI * freqHz / sampleRateHz;
    double cosw0 = cos(w0);
    double sinw0 = sin(w0);
    double alpha = sinw0 / (2.0 * q);

    double b0 = 1.0 + alpha * A;
    double b1 = -2.0 * cosw0;
    double b2 = 1.0 - alpha * A;
    double a0 = 1.0 + alpha / A;
    double a1 = -2.0 * cosw0;
    double a2 = 1.0 - alpha / A;

    storeNormalized(b0, b1, b2, a0, a1, a2, coeffs);
}

void biquadBypass(double coeffs[5]) {
    coeffs[0] = 1.0;
    coeffs[1] = 0.0;
    coeffs[2] = 0.0;
    coeffs[3] = 0.0;
    coeffs[4] = 0.0;
}
