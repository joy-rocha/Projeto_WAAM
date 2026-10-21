**São instruções gravadas diretamente em um chip de memória**

---
### ***PERIFÉRIOCOS UTILIZADOS***

$->$ **microcontrolador usado STM32F446RET6**

==**O microcontrolador STM32F446RET6 não possui suporte nativo para CAN FD.**== Ele integra o periférico conhecido como **bxCAN** (Basic Extended CAN), que oferece suporte exclusivo ao protocolo **CAN 2.0A e CAN 2.0B ativo**.

| ![[Pasted image 20261019130844.png\|130]] | ![[Pasted image 20261020214129.png\|133]] |
| ----------------------------------------- | ----------------------------------------- |

### **USB2CANFD V2  - CANBUS Analyzer**
esse periférico serve de ponte entre o PC e a rede CAN no MCU, permitindo que eles enviem e recebam mensagens entre si

### **WeAct Studio ISOCANFD Module V1**
É o Transceiver CAN isolado do lado do seu microcontrolador (ele usa o chip isolador CA-IS2062A). É ele quem faz a tradução entre os pinos digitais do STM32 e o barramento diferencial do cabo

![[Pasted image 20261021120920.png|637]]

---
# Esquema de funcionamento

![[Pasted image 20261019134850.png|339]]

$-$ o PC manda a mensagem, o USB transforma ela em tensão diferencial, o TRANSCEIVER calcula a diferença e manda o bit de resposta pro STM32

$-$ o STM manda o bit pelo TX, o TRANSCEIVER gera as tensões, o USB lê e converte pro PC.

---
## Cálculo do Baud Rate (500 kbps)
 No CubeMX, você precisa ajustar o _Prescaler_ e os tempos de segmento (_Time Seg1_ e _Time Seg2_) do relógio interno do bxCAN para que a velocidade dê exatamente 500 kbps.     
## Configuração do Filtro de Recepção
Por padrão, o STM32 bloqueia todas as mensagens recebidas até que você configure o filtro (`HAL_CAN_ConfigFilter`). São cerca de 10 linhas de código para dizer ao microcontrolador: _"Aceite qualquer mensagem e jogue na FIFO 0"_.

## Inicialização e Interrupção
- Chamar `HAL_CAN_Start()` para ligar o periférico.
- Chamar `HAL_CAN_ActivateNotification()` para avisar que você quer ser notificado via interrupção.
- Escrever a função de _Callback_ (`HAL_CAN_RxFifo0MsgPendingCallback`) para processar os dados quando um pacote chegar.    

>Com essa estrutura montada, o loop principal (`while(1)`) fica praticamente livre para executar outras tarefas do seu projeto, enquanto o periférico CAN gerencia o envio e a recepção em segundo plano.


---  
# Prioridade de hardware
### configuração de um filtro para gerência de prioridade:

1. **Banco 0 (Alta Prioridade $\rightarrow$ FIFO 1):** Você configura para capturar **apenas os IDs críticos** (exemplo: alertas de emergência, paradas de máquina) e direciona para a **FIFO 1**.

2. **Banco 1 (Geral / Demais Mensagens $\rightarrow$ FIFO 0):** Você configura como **"Passa-Tudo"** e direciona para a **FIFO 0**. Como o Banco 0 já capturou as emergências antes, o Banco 1 só vai receber o restante do tráfego.


```c
// --- BANCO 0: Mensagens de Alta Prioridade (FIFO 1) ---
sFilterConfig.FilterBank = 0;
sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
sFilterConfig.FilterIdHigh = (0x010 << 5);        // Exemplo: ID base de emergência (0x010)
sFilterConfig.FilterIdLow = 0x0000;
sFilterConfig.FilterMaskIdHigh = (0x7F0 << 5);    // Máscara exigindo a faixa 0x010 a 0x01F
sFilterConfig.FilterMaskIdLow = 0x0000;
sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO1; // Vai para a FIFO 1
sFilterConfig.FilterActivation = ENABLE;

HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);

// --- BANCO 1: Todo o Resto do Tráfego (FIFO 0) ---
sFilterConfig.FilterBank = 1;
sFilterConfig.FilterIdHigh = 0x0000;
sFilterConfig.FilterMaskIdHigh = 0x0000;           // Máscara 0 = Passa tudo o que sobrou
sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0; // Vai para a FIFO 0

HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);
```

---
# Esquema de conexão física CAN

![[Pasted image 20261021114512.png|679]]

| Dispositivo Origem | Pino de Origem   | Dispositivo Destino | Pino de Destino | Função / Observação                         |
| ------------------ | ---------------- | ------------------- | --------------- | ------------------------------------------- |
| **USB2CANFD V2**   | H (CAN_H)        | **ISOCANFD V1**     | H (CAN_H)       | Linha diferencial alta — conexão direta     |
| **USB2CANFD V2**   | L (CAN_L)        | **ISOCANFD V1**     | L (CAN_L)       | Linha diferencial baixa — conexão direta    |
| **USB2CANFD V2**   | GND              | **ISOCANFD V1**     | G1 (GND_ISO)    | Referência do barramento isolado (opcional) |
| **STM32F446RET6**  | 5V               | **ISOCANFD V1**     | 5V              | Alimentação do gerador isolado DC-DC        |
| **STM32F446RET6**  | 3V3              | **ISOCANFD V1**     | VIO             | Referência da tensão lógica (3,3 V)         |
| **STM32F446RET6**  | GND              | **ISOCANFD V1**     | GND             | Terra lógico da CPU / microcontrolador      |
| **STM32F446RET6**  | PA12 (`CAN1_TX`) | **ISOCANFD V1**     | TX              | Transmissão de dados — conexão direta       |
| **STM32F446RET6**  | PA11 (`CAN1_RX`) | **ISOCANFD V1**     | RX              | Recepção de dados — conexão direta          |

---
# Configuração no CubeMX

| ![[Pasted image 20261021121830.png\|321]] | ![[Pasted image 20261021121843.png\|348]] |
| ----------------------------------------- | ----------------------------------------- |

### Clock Configuration

|Parâmetro|Valor|
|---|---|
|HCLK|180 MHz|
|APB1 Prescaler|/4|
|PCLK1 (APB1 Clock)|45 MHz|
|APB2 Prescaler|/2|
|Fonte de clock|HSE → PLL|

### Pinout

|Pino do STM32|Função|Descrição|
|---|---|---|
|PA11|CAN1_RX|Recepção de dados CAN|
|PA12|CAN1_TX|Transmissão de dados CAN|
|PA5 (opcional)|GPIO_Output|Indicação de atividade por LED|

### CAN1 — Parameter Settings

|Parâmetro|Valor|
|---|---|
|Prescaler (for Time Quantum)|9|
|Time Quanta in Bit Segment 1|15 Times|
|Time Quanta in Bit Segment 2|4 Times|
|ReSynchronization Jump Width|1 Time|
|Time Triggered Mode|Disable|
|Automatic Bus-Off Management|Enable|
|Automatic Wake-Up Mode|Disable|
|Automatic Retransmission|Enable|
|Receive FIFO Locked Mode|Disable|
|Transmit FIFO Priority|Disable|
|Operating Mode|Normal|

### Cálculo da taxa de transmissão

|Parâmetro|Cálculo|Resultado|
|---|---|---|
|Clock CAN1|PCLK1|45 MHz|
|Frequência após prescaler|45 MHz / 9|5 MHz|
|Time Quantum (TQ)|1 / 5 MHz|200 ns|
|TQ por bit|1 + 15 + 4|20 TQ|
|Taxa de transmissão|45 MHz / (9 × 20)|250 kbps|
|Ponto de amostragem|(1 + 15) / 20 × 100|80%|

### NVIC Settings

|Configuração|Valor|
|---|---|
|CAN1 RX0 interrupts|Enable|
|Prioridade da interrupção|Conforme a aplicação|
|CAN1 TX interrupts|Opcional|
|CAN1 RX1 interrupts|Opcional|

### Resumo do projeto

| Característica           | Valor             |
| ------------------------ | ----------------- |
| Microcontrolador         | STM32F446RET6     |
| Interface                | CAN1              |
| Protocolo                | CAN 2.0A          |
| Identificador            | Padrão de 11 bits |
| Taxa de transmissão      | 250 kbps          |
| Clock APB1               | 45 MHz            |
| Prescaler CAN            | 9                 |
| TQ por bit               | 20                |
| Ponto de amostragem      | 80%               |
| Modo de operação         | Normal            |
| Recepção por interrupção | FIFO 0            |

---



