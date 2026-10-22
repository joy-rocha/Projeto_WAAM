
| ![[Pasted image 20261022073634.png\|218]] |
| ----------------------------------------- |

***ESPECIFICAÇÕES:***
- Controle de direção e velocidade (PWM);  
- Tensão de alimentação: 5 - 27 Vdc;  
- Tensão Lógica: 3,3 - 5,3 V;  
- Corrente de Alimentação (módulo): 3 mA;  
- Máxima potência dissipada: Limitada internamente;
- Máxima corrente continua: 43,0 A;  
- Máxima corrente de pico: 60,0 A;

---
# Lógica de Funcionamento da Ponte H (BTS7960)

A ponte H é uma estrutura de circuito composta por quatro chaves semicondutoras (no BTS7960, transistores de potência/MOSFETs integrados) organizadas na forma da letra "H". Ela permite inverter a polaridade da tensão aplicada ao motor DC sem a necessidade de chaves mecânicas ou relés, controlando o sentido de rotação e a velocidade através da técnica de PWM (Modulação por Largura de Pulso).

O módulo comercial BTS7960 (conhecido como IBT-2) é composto por dois CIs meia-ponte (half-bridge) integrados. Seu funcionamento lógico é estruturado da seguinte forma:

1. **Habilitação dos Canais (`R_EN` e `L_EN`):**
    
    - **`R_EN` (Right Enable):** Habilita o canal direito da ponte.
        
    - **`L_EN` (Left Enable):** Habilita o canal esquerdo da ponte.
        
    - Ambos devem estar em nível lógico alto (**+5V**) para que a ponte H opere. Ao conectá-los ao **+5V** do circuito, o módulo permanece continuamente ativado para receber comandos de PWM.
        
2. **Direção e Velocidade (`RPWM` e `LPWM`):**
    
    - **Sentido Direto (Horário):** Aplica-se o sinal **PWM no pino `RPWM`** e mantém-se o pino **`LPWM` em nível baixo (0V)**. O ciclo de trabalho (duty cycle) do PWM define a velocidade do motor.
        
    - **Sentido Inverso (Anti-horário):** Aplica-se o sinal **PWM no pino `LPWM`** e mantém-se o pino **`RPWM` em nível baixo (0V)**.
        
    - **Parada / Desligado:** Ao aplicar 0% de PWM tanto em `RPWM` quanto em `LPWM`, a corrente deixa de ser fornecida e o motor para por inércia.

|**R_EN**|**L_EN**|**Sinal RPWM**|**Sinal LPWM**|**Estado do Motor**|
|---|---|---|---|---|
|5V|5V|**PWM**|0V|Rotação em Sentido 1 (Velocidade proporcional ao Duty Cycle)|
|5V|5V|0V|**PWM**|Rotação em Sentido 2 (Velocidade proporcional ao Duty Cycle)|
|5V|5V|0V|0V|Motor Desligado / Frenagem passiva|
|0V|0V|X|X|Ponte H Desabilitada (Saídas em alta impedância)|

---
# Configuração no Cube MX

A configuração no CubeMX foi de 20kHz no TIM4, com o clk de 100Hz (Máxima)

**Cálculo:** $\frac{100.000.000\text{ Hz}}{(0 + 1) \cdot (4999 + 1)} = \frac{100.000.000}{5000} = 20.000\text{ Hz } (20\text{ kHz})$

_Com essa configuração, seu controle de velocidade (Duty Cycle) vai de **0 (0%) a 5000 (100%)**._

---
# Tabla de Conexão Física


### STM32F412RET6 $\rightarrow$ BTS7960

|**Pino do STM32F412RET6**|**Conector J5 (Esquema) PDF**|**Pino no Módulo BTS7960**|**Função do Sinal**|
|---|---|---|---|
|**PB6** (Pino 36)|Pino 3 (`PH_RPWM`)|**RPWM**|Sinal PWM para controle de rotação no Sentido 1|
|**PB7** (Pino 33)|Pino 4 (`PH_LPWM`)|**LPWM**|Sinal PWM para controle de rotação no Sentido 2|
|**+5V** (Linha do Buck 5V)|Pino 1 (`+5V`)|**VCC**, **R_EN**, **L_EN**|Alimentação lógica (+5V) e habilitação constante dos canais|
|**GND**|Pino 2 (`GND`)|**GND**|Referência de terra da lógica|

_Nota: Os pinos de medição de corrente do módulo (`R_IS` e `L_IS`) podem ser deixados desconectados caso não vá utilizar leitura analógica de corrente da ponte H diretamente no MCU._

  
### (Fonte 24V $\rightarrow$ BTS7960 $\rightarrow$ Motor DC)

|**Terminal de Potência BTS7960**|**Origem / Destino**|**Descrição**|
|---|---|---|
|**B+ / VCC_PWR**|Fonte de Alimentação (+24V)|Linha principal de alimentação de potência do motor|
|**B- / GND_PWR**|Fonte de Alimentação (GND / 0V)|Retorno do negativo de potência da fonte|
|**M+ / M_L / OUT1**|Terminal 1 do Motor DC|Saída da ponte H conectada a um dos pólos do motor|
|**M- / M_R / OUT2**|Terminal 2 do Motor DC|Saída da ponte H conectada ao outro pólo do motor|
> **Atenção (Terra Comum):** Conecte um fio partindo de um dos bornes **GND da Fonte** diretamente para o pino **GND do STM32**. Isso é indispensável para que a lógica de controle e a potência compartilhem a mesma referência elétrica.

---
# Esquema de conexões físicas do sistema de controle do motor


| ![[Pasted image 20261022091833.png\|366]] |
| ----------------------------------------- |

---
# Funções de controle

```c
// 100% de velocidade no Sentido 1 (RPWM)
__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 5000);
__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);

// 50% de velocidade no Sentido 2 (LPWM)
__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 2500);
```

---

