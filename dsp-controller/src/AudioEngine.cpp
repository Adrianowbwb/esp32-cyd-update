#include "AudioEngine.h"
#include "Biquad.h"
#include "DSPMap.h"

void AudioEngine::applyMasterVolumeMute(const Settings &s) {
    for (uint8_t ch = 0; ch < NUM_CH; ch++) {
        _dsp.setVolumeDb(MASTER_VOL_ADDR[ch], s.masterVolumeDb, s.masterMuted);
    }
}

void AudioEngine::applyCrossover(const Settings &s) {
    double lp[5];
    double hp[5];
    // Linkwitz-Riley 4ª ordem = 2 biquads Butterworth (Q=0.7071) idênticos
    // em cascata, tanto para o passa-baixa quanto para o passa-alta.
    biquadLowpass(s.xoverFreqHz, BUTTERWORTH_Q, DSP_SAMPLE_RATE_HZ, lp);
    biquadHighpass(s.xoverFreqHz, BUTTERWORTH_Q, DSP_SAMPLE_RATE_HZ, hp);

    for (uint8_t ch = 0; ch < NUM_CH; ch++) {
        _dsp.writeFixedBlock(XOVER_LP_ADDR[ch][0], lp, 5);
        _dsp.writeFixedBlock(XOVER_LP_ADDR[ch][1], lp, 5);
        _dsp.writeFixedBlock(XOVER_HP_ADDR[ch][0], hp, 5);
        _dsp.writeFixedBlock(XOVER_HP_ADDR[ch][1], hp, 5);
    }
}

void AudioEngine::applyEqBand(const Settings &s, uint8_t bandIndex) {
    if (bandIndex >= EQ_NUM_BANDS) return;
    double coeffs[5];
    EqBand band = s.eq[bandIndex]; // cópia: campos de struct "packed" podem não estar alinhados
    biquadPeakingEQ(band.freqHz, band.q, band.gainDb, DSP_SAMPLE_RATE_HZ, coeffs);

    for (uint8_t ch = 0; ch < NUM_CH; ch++) {
        _dsp.writeFixedBlock(EQ_ADDR[ch][bandIndex], coeffs, 5);
    }
}

void AudioEngine::applyTrims(const Settings &s) {
    for (uint8_t ch = 0; ch < NUM_CH; ch++) {
        _dsp.setVolumeDb(GAIN_LOW_ADDR[ch], s.gainLowDb, false);
        _dsp.setVolumeDb(GAIN_HIGH_ADDR[ch], s.gainHighDb, false);
    }
}

void AudioEngine::applyDelay(const Settings &s) {
    for (uint8_t ch = 0; ch < NUM_CH; ch++) {
        _dsp.setDelaySamples(DELAY_LOW_ADDR[ch], s.delayLowSamples);
        _dsp.setDelaySamples(DELAY_HIGH_ADDR[ch], s.delayHighSamples);
    }
}

void AudioEngine::applyAll(const Settings &s) {
    applyMasterVolumeMute(s);
    applyCrossover(s);
    for (uint8_t b = 0; b < EQ_NUM_BANDS; b++) {
        applyEqBand(s, b);
    }
    applyTrims(s);
    applyDelay(s);
}
