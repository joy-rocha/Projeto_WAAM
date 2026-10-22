
**encoder magnetico xinXuanSmart magn Tek 6701-ACD**

| ![[Pasted image 20261022103423.png\|226]] |
| ----------------------------------------- |


>É um encoder magnético angular sem contato. Ele mede a posição de um motor ou eixo calculando a orientação do campo magnético de um ímã posicionado logo acima do chip.

### Como Funciona
Dentro do circuito integrado MT6701 existem múltiplos sensores Hall dispostos circularmente. Um ímã com **magnetização diametral** (onde os polos Norte e Sul estão divididos pelas laterais do disco, e não no topo/fundo) é fixado no eixo que vai girar, conforme o ímã gira a uma distância entre **0,5 mm e 2,0 mm** do chip, o MT6701 mede a direção do vetor magnético e calcula o ângulo exato de 0° a 360° com alta precisão (resolução de até 14 bits / 16.384 posições por volta).

---
### Mapeamento dos Pinos da Placa (XinXuanSmart)

|**Pino**|**Localização na Placa**|**Função / Descrição**|
|---|---|---|
|**VCC**|Conectores Esquerdo e Direito|Alimentação elétrica do módulo (suporta de 3,3 V a 5 V).|
|**GND**|Conectores Esquerdo e Direito|Ligação à massa / terra (0 V).|
|**OUT**|Conector Esquerdo|Saída analógica ou sinal PWM proporcional ao ângulo medido (0 V a VCC corresponde a 0° a 360°).|
|**SCL**|Zona Central (I2C)|Linha de relógio (_clock_) para a comunicação digital I2C.|
|**SDA**|Zona Central (I2C)|Linha de dados para a comunicação digital I2C (leitura de posição absoluta de 14 bits).|
|**A**|Conector Direito|Sinal da fase A para o modo de encoder incremental (quadratura).|
|**B**|Conector Direito|Sinal da fase B para o modo de encoder incremental (quadratura).|
|**Z**|Conector Direito|Sinal de índice (_Index_ / _Zero_), emitindo um impulso por cada volta completa.|

---
### Modos de Operação Principais
O MT6701 permite ler a posição de três formas principais:

1. **Modo I2C (Ângulo Absoluto Digital - Recomendado):**
      
    - **Conexão:** `VCC`, `GND`, `SDA`, `SCL`.
    - **Endereço I2C Padrão:** `0x06`.
    - **Vantagem:** Leitura exata da posição absoluta de 14 bits (16.384 passos) via software (usando bibliotecas como SimpleFOC ou drivers MT6701 para Arduino/ESP32).

2. **Modo ABZ (Encoder Incremental):**
     
    - **Conexão:** `VCC`, `GND`, `A`, `B`, `Z`.
    - **Vantagem:** Funciona como um encoder óptico quadratura tradicional para medir velocidade, rotação e direção usando interrupções do microcontrolador.

3. **Modo Analógico / PWM:**
    
    - **Conexão:** `VCC`, `GND`, `OUT`.
    - **Vantagem:** O pino `OUT` entrega uma tensão contínua proporcional ao ângulo, que pode ser lida em um pino ADC do microcontrolador.


---
# Requisitos para um bom funcionamento

- **Ímã Correto:** **Não utilize ímãs de neodímio comuns com polarização axial** (onde a face de cima é N e a de baixo é S). É obrigatório utilizar um ímã com **polarização diametral**.

- **Alinhamento Mecânico:** O centro do ímã deve ficar alinhado verticalmente com o centro do chip MT6701, com uma folga (gap) de cerca de **1 mm a 1,5 mm**.

- **Resistores de Pull-up:** Se for utilizar a interface I2C, certifique-se de que os pinos `SDA` e `SCL` do seu microcontrolador (ou da placa) possuam resistores de pull-up para VCC (geralmente de 4.7kΩ).

--- 

# Conexões físicas do monitoramento de posição do eixo do motor

>Para um **Motor DC**, o modo de funcionamento mais indicado é o **Modo ABZ (Encoder Incremental por Quadratura)**. O STM32F412RET6 possui periféricos de Timer com suporte nativo por hardware (_Encoder Mode_), o que lê a rotação e a direção do motor sem ocupar o processador.

### Modo ABZ 
_Utiliza o Timer 2 (TIM2) do STM32 em modo Encoder por Hardware:_

|**Pino do Encoder (MT6701)**|**Conector na Placa**|**Pino no STM32F412RET6**|**Periférico / Função Correta no STM32**|
|---|---|---|---|
|**VCC**|Pino 1 de J4|**3.3V / 5V**|Alimentação elétrica|
|**GND**|Pino 4 de J4|**GND**|Terra de referência|
|**A**|Pino 2 de J4|**PA0**|**`TIM2_CH1`** (Fase A do Encoder)|
|**B**|Pino 3 de J4|**PA1**|**`TIM2_CH2`** (Fase B do Encoder)|
|**Z**|_Fora do conector J4_|**PB0**|**`GPIO_EXTI0`** (Interrupção de volta completa)|

---


![[Pasted image 20261022134124.png|424]]

---
