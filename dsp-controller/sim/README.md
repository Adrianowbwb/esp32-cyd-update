# Simulador de terminal

Roda a lógica de controle do firmware (menu, encoder, cálculo de
coeficientes de biquad, persistência em "EEPROM") direto no PC, sem
precisar de ATmega328PB, encoder, LCD ou ADAU1701 físicos.

**O que É simulado:** toda a lógica de `src/UI.cpp`, `src/AudioEngine.cpp`,
`src/Biquad.cpp` e `src/Settings.cpp` — ou seja, exatamente o código que
roda no microcontrolador de verdade. O LCD 16x2 vira uma caixinha ASCII no
terminal, o encoder vira teclas do teclado, e a EEPROM vira o arquivo
`sim_eeprom.bin` (persiste entre execuções, igual a EEPROM de verdade).

**O que NÃO é simulado:** o processamento de áudio dentro do ADAU1701. As
escritas que iriam por I2C para o chip só são logadas em texto (endereço +
valor), para você conferir que os cálculos de crossover/EQ/delay/volume
estão corretos. Áudio de verdade só existe com o chip físico rodando o
programa do SigmaStudio.

## Compilar e rodar

```bash
cd dsp-controller/sim
./build.sh
./dsp_sim
```

Requer só um `g++` comum (qualquer Linux/macOS com toolchain C++14) — não
precisa do PlatformIO nem de um toolchain AVR.

## Controles (teclado no lugar do encoder)

| Tecla       | Ação equivalente no encoder real |
|-------------|-----------------------------------|
| `a`         | Girar 1 passo anti-horário |
| `d`         | Girar 1 passo horário |
| `A`         | Girar 3 passos anti-horário (ajuste rápido) |
| `D`         | Girar 3 passos horário (ajuste rápido) |
| `espaço`    | Clique curto (selecionar/confirmar) |
| `l`         | Pressão longa (voltar/cancelar) |
| `q`         | Sair do simulador |

A tela mostra as duas linhas do LCD, a última escrita que teria ido para
o DSP via I2C, e os controles disponíveis. Navegue pelo mesmo menu descrito
no `README.md` principal (Volume, Mute, Xover Freq, EQ1-3 Freq/Gain/Q,
Trim Grave/Agudo, Delay Grave/Agudo, Salvar e sair).
