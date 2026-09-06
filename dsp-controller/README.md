# Controlador DSP ADAU1701 com ATmega328PB

Firmware que transforma um ATmega328PB + encoder rotativo + display LCD
16x2 em um controle remoto físico para um DSP Analog Devices ADAU1701,
para uso em um crossover ativo estéreo com EQ paramétrico, delay,
ganho/trim por via, volume master, mute e LEDs indicadores.

## O que o firmware controla

Menu único, navegado 100% pelo encoder (girar = navegar/ajustar, clique
curto = confirmar, clique longo = voltar/cancelar):

| Item             | Faixa                          | Efeito no DSP |
|------------------|---------------------------------|----------------|
| Volume           | -60.0 a 0.0 dB (passo 0.5dB)     | Volume master (L+R) |
| Mute             | ON / OFF                        | Mute master (L+R) |
| Xover Freq       | 20 Hz a 20 kHz (passo musical 1/12 oitava) | Frequência de corte do crossover Linkwitz-Riley 24dB/oitava |
| EQ1/EQ2/EQ3 Freq | 20 Hz a 20 kHz                   | Frequência central de cada banda paramétrica |
| EQ1/EQ2/EQ3 Gain | -12 a +12 dB (passo 0.5dB)       | Ganho da banda (boost/cut) |
| EQ1/EQ2/EQ3 Q    | 0.30 a 10.0 (passo 0.05)         | Fator Q (largura) da banda |
| Trim Grave       | -12 a +12 dB                    | Ganho de nível da via grave (pós-crossover) |
| Trim Agudo       | -12 a +12 dB                    | Ganho de nível da via aguda (pós-crossover) |
| Delay Grave      | 0 a ~85 ms                      | Delay da via grave (alinhamento temporal) |
| Delay Agudo      | 0 a ~85 ms                      | Delay da via aguda (alinhamento temporal) |
| Salvar e sair    | —                                | Grava tudo na EEPROM e volta à tela inicial |

Todos os controles são "linkados" (mesmo valor aplicado a L e R). Os
endereços de célula L e R são escritos separadamente (ver `DSPMap.h`),
então é simples evoluir para controle independente por canal se precisar.

LEDs:

- **Power** — aceso sempre que o firmware está rodando.
- **Mute** — aceso enquanto o mute master está ativo.
- **Clip** — pisca (~400ms) quando o detector de nível do DSP acusa um
  pico acima do limiar configurado em `DSP_CLIP_THRESHOLD_FIXED`.
- **Signal** — saída disponível em `LedIndicators::setSignal()`, pronta
  para você ligar a uma segunda célula "Level Detect" com limiar baixo,
  indicando presença de áudio (não vem acionada por padrão).

Configurações são persistidas na EEPROM interna do ATmega328PB (com CRC8
de verificação) e reaplicadas no DSP automaticamente a cada boot.

## Hardware necessário

- ATmega328PB (DIP28, TQFP32 ou QFN32) com cristal/ressonador externo de
  16 MHz + capacitores de 22pF, ou use um ressonador cerâmico de 3 pinos.
- Módulo ADAU1701 com EEPROM de boot (24LC32 ou similar) — os módulos
  chineses de baixo custo já vêm assim.
- Encoder rotativo incremental com botão (KY-040/EC11).
- Display LCD 16x2 + backpack I2C PCF8574 (endereço `0x27` por padrão).
- 4 LEDs + resistores de ~330-470Ω.
- Programador ISP (USBasp, USBtinyISP, Arduino como ISP, etc.) — o
  ATmega328PB "puro" não vem com bootloader Arduino de fábrica.

## Fiação

| Sinal                | Pino ATmega328PB (nomenclatura Arduino) |
|-----------------------|------------------------------------------|
| Encoder A             | D2 (INT0) |
| Encoder B             | D3 (INT1) |
| Encoder SW (botão)    | D4 (pull-up interno, ativo em LOW) |
| I2C SDA (DSP + LCD)   | A4 |
| I2C SCL (DSP + LCD)   | A5 |
| LED Power             | D5 |
| LED Mute              | D6 |
| LED Clip              | D7 |
| LED Signal            | D8 |

DSP e LCD ficam no **mesmo barramento I2C**, em endereços diferentes
(`0x34` o ADAU1701, `0x27` o backpack do LCD) — use resistores de pull-up
de 4.7kΩ em SDA/SCL se o seu módulo ADAU1701 não já os tiver.

Todos os pinos podem ser remapeados em `include/Config.h`.

## Preparando o projeto no SigmaStudio

Este firmware **não substitui o SigmaStudio** — ele controla, via I2C, um
programa de DSP que você já compilou e gravou no ADAU1701 usando o
SigmaStudio. O schematic esperado é:

```
Entrada estéreo
   |
   +-- EQ paramétrico L (3x "General 2nd Order" em série)
   +-- EQ paramétrico R (3x "General 2nd Order" em série)
        |
        +---- LPF via grave (2x "General 2nd Order" em cascata = LR4) --> Volume/trim grave --> Delay grave --> Saída grave
        |
        +---- HPF via aguda (2x "General 2nd Order" em cascata = LR4) --> Volume/trim agudo --> Delay agudo --> Saída aguda
   (repita para L e R)
   |
   +-- Volume/Mute master (por canal, célula "Volume" com mute embutido)
   +-- (opcional) "Level Detect" por canal, para o LED de clip
```

Passo a passo:

1. Monte esse schematic no SigmaStudio (Estúdio → New Project), com a
   taxa de amostragem que você quer usar (por padrão o firmware assume
   48 kHz — ajuste `DSP_SAMPLE_RATE_HZ` em `DSPMap.h` se usar outra).
2. Para cada célula "General 2nd Order" (biquad), deixe os coeficientes
   no modo padrão — eles serão sobrescritos pelo firmware em tempo real,
   os valores iniciais no SigmaStudio não importam.
3. Nas células de "Volume", habilite a opção de mute controlável via
   registrador (para a célula de volume master).
4. Na célula de "Delay", aloque memória suficiente para o delay máximo
   que você quer permitir (o padrão do firmware é `DSP_MAX_DELAY_SAMPLES
   = 4096` amostras ≈ 85 ms @ 48 kHz — ajuste os dois lados juntos).
5. Compile (Link. Compile. Download) e grave o programa na EEPROM externa
   do módulo (self-boot), ou baixe direto no chip durante o
   desenvolvimento.
6. Menu **Actions → Export System Files**. Abra o arquivo
   `<projeto>_IC_1_PARAM.h` gerado.
7. Para cada bloco (EQ1L, EQ1R, XoverLP_L, Volume master L, Delay grave
   L, etc.), copie o `#define ..._ADDR` correspondente para
   `include/DSPMap.h`, nas constantes já organizadas lá
   (`EQ_ADDR`, `XOVER_LP_ADDR`, `XOVER_HP_ADDR`, `GAIN_LOW_ADDR`,
   `GAIN_HIGH_ADDR`, `DELAY_LOW_ADDR`, `DELAY_HIGH_ADDR`,
   `MASTER_VOL_ADDR`, `MASTER_MUTE_ADDR`, `CLIP_DETECT_ADDR`).
8. Confirme o endereço I2C do seu módulo (`DSP_I2C_ADDR`, normalmente
   `0x34` com o pino ADDR0 em GND).

**Os valores em `DSPMap.h` neste repositório são placeholders** — sem o
passo 6/7 acima, o firmware vai escrever nos endereços errados do seu
projeto específico.

## Formato de dados do ADAU1701

Os registradores de coeficiente/volume usam ponto fixo 5.23 (28 bits,
sinal + 4 bits inteiros + 23 fracionários), transmitidos em blocos de 4
bytes MSB-first. `ADAU1701::writeFixed()` faz essa conversão
automaticamente; células de "Delay" usam um inteiro simples (número de
amostras), tratado por `ADAU1701::setDelaySamples()`.

## Compilando e gravando (PlatformIO)

```bash
# Compilar
pio run

# Gravar via programador ISP (ajuste upload_protocol em platformio.ini
# conforme seu programador — usbasp, usbtinyisp, arduinoisp, etc.)
pio run -t upload

# Gravar os fuses (clock externo, BOD, etc.) na primeira gravação
pio run -t fuses
```

Se preferir usar a IDE do Arduino em vez do PlatformIO, instale o core
[MiniCore](https://github.com/MCUdude/MiniCore) (suporte oficial ao
ATmega328PB), selecione a placa "ATmega328PB", 16 MHz externo, e copie o
conteúdo de `include/` e `src/` para uma pasta de sketch (renomeie
`src/main.cpp` para `dsp_controller.ino`).

## Simulador de terminal (sem hardware)

Para testar a lógica de menu/EQ/crossover/delay/persistência sem precisar
de ATmega, encoder, LCD ou DSP físicos:

```bash
cd sim
./build.sh
./dsp_sim
```

Roda a mesma UI/AudioEngine/Biquad/Settings do firmware real, com o LCD
virando uma caixinha no terminal, o encoder virando teclado e a EEPROM
virando um arquivo local. Não simula o áudio dentro do ADAU1701 — só a
lógica de controle. Detalhes em `sim/README.md`.

## Estrutura do código

```
include/
  Config.h       pinos e timings da interface
  DSPMap.h       endereços de célula do SigmaStudio + parâmetros do DSP
  ADAU1701.h     driver I2C de baixo nível (ponto fixo 5.23, delay, readback)
  Biquad.h       cálculo de coeficientes (lowpass/highpass/peaking EQ)
  Settings.h     struct de parâmetros + persistência em EEPROM
  Encoder.h      leitura de quadratura por interrupção + botão
  LEDs.h         LEDs indicadores (power/mute/clip/signal)
  UI.h           máquina de estados do menu no LCD
  AudioEngine.h  traduz Settings -> escritas no DSP
src/
  *.cpp          implementações correspondentes
  main.cpp       setup()/loop()
```

## Extensões possíveis

- Trocar o link estéreo por controle independente por canal (a estrutura
  de endereços já separa L/R, é só desdobrar os itens de menu).
- Crossover de 3 vias (adicionar uma banda "média" com passa-faixa
  formado por LPF+HPF em cascata).
- Múltiplos presets salvos na EEPROM, selecionáveis por um novo item de
  menu.
- Segunda UART do 328PB para debug via serial sem conflitar com I2C.
