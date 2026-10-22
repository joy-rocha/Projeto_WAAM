
---
### 1. Declarações Globais (Variáveis de Memória)

Este bloco reserva os espaços na memória RAM que a biblioteca HAL e o microcontrolador usarão para montar e ler os pacotes.

- **`CAN_HandleTypeDef hcan1;`**: Objeto principal que representa o periférico bxCAN1 do STM32.

- **Bloco Transmissão (TX):**
      
    - **`TxHeader`**: A "etiqueta" do pacote que o STM32 vai enviar (guarda o ID, tamanho e formato).
        
    - **`TxData[8]`**: Os 8 bytes com o conteúdo real da mensagem enviada.
        
    - **`TxMailbox`**: A caixa de saída do hardware onde a mensagem fica aguardando o momento exato de ir para o cabo.  

- **Bloco Recepção (RX) — Dupla FIFO:**
    
	- **`RxHeader0` / `RxData0[8]`**: Etiqueta e dados reservados para mensagens do **tráfego geral** (FIFO 0).
        
    - **`RxHeader1` / `RxData1[8]`**: Etiqueta e dados reservados exclusivamente para **mensagens de emergência/alta prioridade** (FIFO 1).

---
### 2. Inicialização do Sistema e Periféricos

No início da `main()`, o microcontrolador acorda e prepara os circuitos internos.

- **`HAL_Init()`**: Reseta os periféricos e inicia o temporizador de milissegundos (`SysTick`).
- **`SystemClock_Config()`**: Configura a velocidade do processador (frequência dos cristais).
- **`MX_GPIO_Init()` / `MX_CAN1_Init()`**: Funções geradas pelo CubeMX para energizar os pinos e configurar as métricas de tempo da rede CAN (frequência, _prescaler_ e segmentos de tempo). 

---
### 3. Configuração dos Filtros de Hardware (Bancos e Prioridades)

Aqui é definida a regra de entrada das mensagens nos filtros físicos do chip, direcionando o tráfego antes de acordar a CPU.


- **Banco 0 (Alta Prioridade $\rightarrow$ FIFO 1):**
      
    - Configurado com **`FilterBank = 0`** e **`FilterFIFOAssignment = CAN_RX_FIFO1`**.
        
    - Usando `FilterIdHigh` e `FilterMaskIdHigh`, ele exige que a mensagem tenha uma faixa de ID específica (ex: de `0x010` a `0x01F`) para ser direcionada à fila prioritária (FIFO 1).


- **Banco 1 (Tráfego Geral / Passa-Tudo $\rightarrow$ FIFO 0):**
    
    - Configurado com **`FilterBank = 1`** e **`FilterFIFOAssignment = CAN_RX_FIFO0`**.  
        
    - Com a máscara zerada (`FilterMaskIdHigh = 0x0000`), ele funciona como "passa-tudo" para qualquer outra mensagem normal que sobrou do Banco 0.

---
### 4. Partida da CAN e Ativação de Interrupções

Com o filtro pronto, este bloco liga o módulo e autoriza os avisos de chegada.

- **`HAL_CAN_ConfigFilter(...)`**: Grava as regras dos Bancos 0 e 1 nos registradores do chip.

- **`HAL_CAN_Start(&hcan1)`**: Coloca o periférico CAN1 em modo ativo no barramento físico.

- **`HAL_CAN_ActivateNotification(...)`**: Habilita o sistema de interrupções para avisar a CPU quando chegar mensagem na FIFO 0 **OU** na FIFO 1.

- **Definição do `TxHeader`**: Preenche a etiqueta fixa para os envios do STM32 (ID `0x100`, protocolo CAN 2.0A de 11 bits e tamanho de 8 bytes).

---
### 5. Laço Principal (`while (1)`) — Envio Periódico

O ciclo que roda continuamente enquanto a placa estiver ligada.

- **Preenchimento do `TxData[]`**: Atualiza os 8 bytes do vetor com os dados atualizados (no exemplo, `0x01` até `0x08`).

- **`HAL_CAN_AddTxMessage(...)`**: Copia o pacote para uma caixa de saída da CAN para ser transmitido ao barramento físico.

- **`HAL_Delay(1000)`**: Pausa a transmissão por 1 segundo (1000 ms) antes de repetir o laço.

---
### 6. Callbacks de Interrupção (Recepção Automática)

Funções executadas de forma assíncrona pelo hardware quando um pacote chega da rede.

- **`HAL_CAN_RxFifo0MsgPendingCallback`**: Disparada automaticamente quando chega uma mensagem na FIFO 0 (tráfego geral). Chama `HAL_CAN_GetRxMessage` para extrair os dados e processar comandos normais.
    
- **`HAL_CAN_RxFifo1MsgPendingCallback`**: Disparada imediatamente quando chega uma mensagem na FIFO 1 (alta prioridade). Permite reagir de forma instantânea a paradas de emergência sem passar pela fila comum.

---
### 7. Trava de Erro (`Error_Handler`)

- **`Error_Handler()`**: Se qualquer etapa de inicialização (filtro, partida ou interrupção) falhar, a placa desativa as interrupções (`__disable_irq()`) e entra em um laço infinito travado por segurança.

---
