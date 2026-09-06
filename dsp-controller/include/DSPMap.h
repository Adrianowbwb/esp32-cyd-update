// DSPMap.h
// -----------------------------------------------------------------------
// Endereçamento do projeto SigmaStudio para o ADAU1701.
//
// IMPORTANTE: os endereços de célula (cell address) abaixo são
// ESPECÍFICOS de cada projeto SigmaStudio — eles mudam se você adicionar,
// remover ou reordenar blocos no schematic. Os valores aqui são apenas
// placeholders organizados para o layout de DSP descrito no README
// (crossover estéreo 2 vias + EQ paramétrico + delay + trims + master).
//
// Como obter os endereços reais:
//   1. Monte o schematic no SigmaStudio conforme o README.md.
//   2. Menu "Actions" -> "Export System Files".
//   3. Abra o arquivo "<projeto>_IC_1_PARAM.h" gerado. Cada define
//      "MOD_xxxAlgxxx_ADDR" corresponde ao endereço de uma célula.
//   4. Copie os valores para as constantes correspondentes abaixo.
//   5. Se você renomear as células no SigmaStudio para os nomes usados
//      neste header (EQ_L1, EQ_L2, ..., XOVER_LP_L1, ...), fica muito mais
//      fácil localizar cada define no arquivo exportado.
// -----------------------------------------------------------------------

#pragma once

#include <stdint.h>

// Endereço I2C (7 bits) do ADAU1701. Depende do strap do pino ADDR0:
// GND -> 0x34 (padrão de fábrica / self-boot), VCC -> 0x35.
#define DSP_I2C_ADDR        0x34

// Taxa de amostragem configurada no SigmaStudio (Hz). Necessária para
// converter Hz/ms em coeficientes de biquad e em samples de delay.
#define DSP_SAMPLE_RATE_HZ  48000UL

// Memória de delay alocada por bloco de delay no SigmaStudio (em amostras).
// Define o valor MÁXIMO que o encoder pode ajustar. Ajuste conforme o
// "Delay Memory" configurado em cada célula (Tools -> ... -> RAM usada).
#define DSP_MAX_DELAY_SAMPLES  4096   // ~85ms @ 48kHz

// Canais
#define CH_L 0
#define CH_R 1
#define NUM_CH 2

// Número de bandas do EQ paramétrico (por canal, antes do crossover)
#define EQ_NUM_BANDS 3

// -----------------------------------------------------------------------
// EQ paramétrico (Parametric EQ / "General 2nd Order" cells), 1 biquad
// por banda e por canal. eqAddr[canal][banda]
// -----------------------------------------------------------------------
static const uint16_t EQ_ADDR[NUM_CH][EQ_NUM_BANDS] = {
    /* CH_L */ { 0x0000, 0x0005, 0x000A },
    /* CH_R */ { 0x000F, 0x0014, 0x0019 },
};

// -----------------------------------------------------------------------
// Crossover Linkwitz-Riley de 4ª ordem (24 dB/oitava), implementado como
// 2 biquads Butterworth (Q=0.7071) em cascata por via/canal.
// xoverAddr[canal][estágio 0 ou 1]
// -----------------------------------------------------------------------
static const uint16_t XOVER_LP_ADDR[NUM_CH][2] = {
    /* CH_L */ { 0x001E, 0x0023 },
    /* CH_R */ { 0x0028, 0x002D },
};
static const uint16_t XOVER_HP_ADDR[NUM_CH][2] = {
    /* CH_L */ { 0x0032, 0x0037 },
    /* CH_R */ { 0x003C, 0x0041 },
};

// -----------------------------------------------------------------------
// Ganho (trim) por via, célula "Single/Dual Volume" com slew, em dB.
// -----------------------------------------------------------------------
static const uint16_t GAIN_LOW_ADDR[NUM_CH]  = { 0x0046, 0x0047 };
static const uint16_t GAIN_HIGH_ADDR[NUM_CH] = { 0x0048, 0x0049 };

// -----------------------------------------------------------------------
// Delay por via (célula "Delay", endereço único, comprimento em amostras)
// -----------------------------------------------------------------------
static const uint16_t DELAY_LOW_ADDR[NUM_CH]  = { 0x004A, 0x004B };
static const uint16_t DELAY_HIGH_ADDR[NUM_CH] = { 0x004C, 0x004D };

// -----------------------------------------------------------------------
// Master: volume (com slew) e mute, geralmente uma célula "Volume" com
// controle de mute embutido, uma por canal (linkados na UI).
// -----------------------------------------------------------------------
static const uint16_t MASTER_VOL_ADDR[NUM_CH]  = { 0x004E, 0x004F };
static const uint16_t MASTER_MUTE_ADDR[NUM_CH] = { 0x0050, 0x0051 };

// -----------------------------------------------------------------------
// Detector de nível/clipping (célula "Level Detect" com saída lida via
// readback). Usado só para acender o LED de clip. Se você não tiver essa
// célula no projeto, defina DSP_HAS_CLIP_DETECT como 0.
// -----------------------------------------------------------------------
#define DSP_HAS_CLIP_DETECT 1
static const uint16_t CLIP_DETECT_ADDR[NUM_CH] = { 0x0052, 0x0053 };
// Limiar (formato 5.23) acima do qual consideramos "clip". 0.0dBFS = 1.0
// já em ponto fixo 5.23 é 0x00800000; usamos uma margem de -1dBFS.
#define DSP_CLIP_THRESHOLD_FIXED  0x0072B000UL
