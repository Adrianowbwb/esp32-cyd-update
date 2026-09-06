// Config.h
// Mapa de pinos e parâmetros gerais de hardware do controlador.
// Ajuste aqui para bater com a sua placa.

#pragma once

#include <Arduino.h>

// ---------------------------------------------------------------------
// Encoder rotativo (KY-040 ou similar, com chave de push integrada)
// ---------------------------------------------------------------------
// ENC_A e ENC_B precisam ser pinos com interrupção externa no 328PB/328P:
// D2 = INT0, D3 = INT1.
#define PIN_ENC_A   2
#define PIN_ENC_B   3
#define PIN_ENC_SW  4   // Botão do encoder (INPUT_PULLUP, ativo em LOW)

// ---------------------------------------------------------------------
// Barramento I2C compartilhado: DSP ADAU1701 + display LCD 16x2 (backpack
// PCF8574). A4 = SDA, A5 = SCL (padrão do 328P/328PB).
// ---------------------------------------------------------------------
#define I2C_CLOCK_HZ      100000UL

#define LCD_I2C_ADDR      0x27   // endereço comum dos backpacks PCF8574
#define LCD_COLS          16
#define LCD_ROWS          2

// ---------------------------------------------------------------------
// LEDs indicadores
// ---------------------------------------------------------------------
#define PIN_LED_POWER     5   // aceso sempre que o sistema está operando
#define PIN_LED_MUTE      6   // aceso quando o mute geral está ativo
#define PIN_LED_CLIP      7   // pisca quando detecta clipping no DSP
#define PIN_LED_SIGNAL    8   // aceso quando há sinal de áudio presente

// ---------------------------------------------------------------------
// Timings de interface
// ---------------------------------------------------------------------
#define ENCODER_LONGPRESS_MS   700   // tempo para considerar "long press"
#define ENCODER_DEBOUNCE_MS    3
#define MENU_IDLE_TIMEOUT_MS   8000  // volta pra tela inicial se ficar parado
#define SAVE_DEBOUNCE_MS       1500  // grava na EEPROM X ms após última mudança
#define CLIP_LED_HOLD_MS       400   // tempo que o LED de clip fica aceso após detectar pico

// Tempo de espera no boot antes do ATmega falar com o DSP via I2C, para
// garantir que o auto-boot do ADAU1701 (a partir da EEPROM externa,
// 24LC32 ou similar) já tenha terminado de carregar o programa gerado
// pelo SigmaStudio.
#define DSP_BOOT_DELAY_MS      1000
