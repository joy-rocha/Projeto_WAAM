
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

# Teste do encoder magnético ABZ com STM32F412RET6

Estou assumindo que o seu encoder é o **MT6701 da MagnTek**, porque não encontrei esse "6701-ACD" exato. Os nomes dos pinos variam conforme o encapsulamento ou a placa-módulo, então confira com o datasheet e a serigrafia da sua placa.

## 1. Tabela de conexões

|Encoder (MT6701)|STM32F412RET6 (LQFP64)|Observação|
|---|---|---|
|VDD / VCC|3V3|Alimente com 3,3 V para os sinais A/B/Z ficarem em nível 3,3 V|
|GND|GND|GND comum entre encoder, STM32 e adaptador USB-serial|
|A (A/U)|**PA0** (TIM2_CH1)|Canal A|
|B (B/V)|**PA1** (TIM2_CH2)|Canal B|
|Z (Z/W)|**PB0** (EXTI0)|Pulso de índice (1 por volta)|
|MODE|Conforme o datasheet|Esse pino seleciona ABZ ou UVW. Veja o nível exigido para ABZ|
|DIR (se existir)|GND ou 3V3|Define o sentido de contagem. Não deixe flutuando|
|SCL/SDA/CSN/OUT|Não conectar|Só necessários para configurar a resolução (I2C/SSI)|

**Serial para o Termite** (precisa de um adaptador USB-serial tipo CP2102, CH340 ou FTDI):

|Adaptador USB-serial|STM32F412RET6|
|---|---|
|RX|**PA2** (USART2_TX)|
|TX|**PA3** (USART2_RX)|
|GND|GND|

Se você estiver usando uma placa com ST-Link e Virtual COM Port, os pinos do USART podem ser outros. Confira no esquemático da placa.

## 2. Configuração no CubeMX / CubeIDE

1. **File → New → STM32 Project**, busque **STM32F412RET6** e crie o projeto.
2. **System Core → SYS:** Debug = **Serial Wire**.
3. **System Core → RCC:** se a sua placa tem cristal, coloque HSE = _Crystal/Ceramic Resonator_. Se não tem, deixe como está e use o HSI.
4. **Clock Configuration:** configure o clock conforme a sua placa. 84 MHz ou 100 MHz servem, e o teste nem exige muito clock.
5. **Timers → TIM2:**
    - Combined Channels = **Encoder Mode**
    - Prescaler = **0**
    - Counter Mode = **Up**
    - Counter Period = **0xFFFFFFFF** (o TIM2 é de 32 bits)
    - Encoder Mode = **Encoder Mode TI1 and TI2** (contagem x4)
    - Input Filter (IC1 e IC2) = **4** (ajuda contra ruído)
    - Polarity de IC1 e IC2 = **Rising Edge**
    - Os pinos devem aparecer como PA0 e PA1. Se não, clique neles e atribua TIM2_CH1 e TIM2_CH2.
6. **GPIO → PB0:**
    - Clique em PB0 e escolha **GPIO_EXTI0**.
    - Em System Core → GPIO, configure: Mode = **External Interrupt Mode with Rising edge trigger detection**, Pull = **No pull** (ou Pull-down, se o sinal Z ficar flutuando).
    - Em **NVIC**, habilite **EXTI line0 interrupt**.
7. **Connectivity → USART2:**
    - Mode = **Asynchronous**
    - Baud Rate = **115200**, Word Length 8 bits, Parity None, Stop Bits 1
    - Pinos PA2 (TX) e PA3 (RX)
8. Salve o `.ioc` (Ctrl+S) e clique em **Generate Code**.

## 3. Código de teste (`main.c`)

Cole cada trecho dentro dos blocos `USER CODE` indicados, para o CubeMX não apagar o código depois.

```c
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* USER CODE BEGIN PD */
#define ENCODER_PPR   1024               // pulsos por volta configurados no MT6701 (ABZ)
#define ENCODER_CPR   (ENCODER_PPR * 4)  // contagens por volta (modo x4)
#define SAMPLE_MS     100                // intervalo de leitura/impressão
#define RESET_ON_Z    0                  // 1 = zera o contador no pulso Z
/* USER CODE END PD */

/* USER CODE BEGIN PV */
volatile uint32_t z_count = 0;
volatile uint8_t  z_flag  = 0;
/* USER CODE END PV */

/* USER CODE BEGIN 0 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == GPIO_PIN_0)   // Z em PB0
  {
    z_count++;
    z_flag = 1;
#if RESET_ON_Z
    __HAL_TIM_SET_COUNTER(&htim2, 0);
#endif
  }
}

static void uart_print(const char *s)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)s, strlen(s), 100);
}
/* USER CODE END 0 */
```

Dentro do `main()`, depois dos `MX_..._Init()`:

```c
  /* USER CODE BEGIN 2 */
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
  uart_print("\r\n=== Teste encoder ABZ - MT6701 ===\r\n");

  uint32_t last_tick  = HAL_GetTick();
  uint32_t last_count = __HAL_TIM_GET_COUNTER(&htim2);
  char msg[128];
  /* USER CODE END 2 */
```

E dentro do `while (1)`:

```c
    /* USER CODE BEGIN 3 */
    if (HAL_GetTick() - last_tick >= SAMPLE_MS)
    {
      last_tick += SAMPLE_MS;

      uint32_t cnt   = __HAL_TIM_GET_COUNTER(&htim2);
      int32_t  delta = (int32_t)(cnt - last_count);   // com sinal, trata overflow
      last_count = cnt;

      int32_t pos = (int32_t)cnt;
      int32_t pos_mod = ((pos % ENCODER_CPR) + ENCODER_CPR) % ENCODER_CPR;

      // ângulo em centésimos de grau (evita printf de float)
      int32_t ang_c = (int32_t)(((int64_t)pos_mod * 36000) / ENCODER_CPR);

      // RPM x10 (delta contagens em SAMPLE_MS ms)
      int32_t rpm_x10 = (int32_t)(((int64_t)delta * 600000) / ((int64_t)ENCODER_CPR * SAMPLE_MS));

      snprintf(msg, sizeof(msg),
               "CNT=%10ld | ANG=%3ld.%02ld deg | RPM=%ld.%ld | Z=%lu\r\n",
               (long)pos,
               (long)(ang_c / 100), (long)(ang_c % 100),
               (long)(rpm_x10 / 10), (long)(abs(rpm_x10) % 10),
               (unsigned long)z_count);
      uart_print(msg);

      if (z_flag)
      {
        z_flag = 0;
        uart_print(">>> Pulso Z detectado\r\n");
      }
    }
  }
  /* USER CODE END 3 */
```

Se der erro em `abs`, inclua `#include <stdlib.h>`.

## 4. Configuração do Termite

- Escolha a porta **COM** do adaptador USB-serial.
- Baud rate **115200**, Data bits **8**, Stop bits **1**, Parity **none**, Flow control **none**.
- Clique em **OK** e depois gire o encoder (ou o ímã) para ver os dados.

Saída esperada:

```
CNT=      2048 | ANG=180.00 deg | RPM=12.3 | Z=1
>>> Pulso Z detectado
```

## 5. Problemas comuns

- **Não conta nada:** verifique o pino MODE (se está no nível de ABZ), a alimentação, o GND comum e se o ímã está bem centralizado sobre o chip.
- **Conta ao contrário:** troque os fios A e B, ou mude o pino DIR.
- **Contagem instável ou com saltos:** aumente o filtro do TIM2 (6 a 8), use fios curtos e ímã centralizado e na distância certa.
- **Z não dispara:** confira o PB0, a interrupção EXTI0 no NVIC e se o Z não está só como UVW em outro modo.
- **Ângulo errado:** ajuste `ENCODER_PPR` para a resolução que o MT6701 está realmente configurada. A resolução ABZ é programável na EEPROM do chip (via I2C/SSI), então o valor depende do que está gravado no seu.

Se você me disser qual placa está usando (própria ou de desenvolvimento) e qual o encapsulamento do encoder, eu ajusto os pinos e a configuração de clock para o seu caso.

fgfhh