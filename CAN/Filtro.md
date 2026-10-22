
>O **Filtro de Hardware** funciona como um "segurança": ele fica na entrada do chip analisando os IDs do barramento.

- Se a mensagem não cumpre os requisitos $\rightarrow$ o hardware **descarta** em nanossegundos sem nem acordar o STM32.
- Se a mensagem passa no teste $\rightarrow$ o hardware guarda a mensagem na memória e avisa a CPU (dispara a interrupção).

---
# Filter Bank

>Um **Banco de Filtro** nada mais é do que um **bloco físico de registradores** no chip dedicado a guardar as regras de filtragem.

- O STM32F446RET6 possui **28 Bancos de Filtro** (numerados de 0 a 27).
- Cada banco pode ser configurado de forma independente com regras diferentes.
- Se você ativar o Banco 0 com uma regra e o Banco 1 com outra, uma mensagem que chegue e atenda a **qualquer uma** das regras será aceita.

---
# Modos de Operação
**Cada Banco de Filtro pode operar em dois modos distintos:**

#### Modo Lista (Identifier List Mode)
>Você cadastra explicitamente quais IDs quer receber.

- **Exemplo:** "Só quero aceitar as mensagens com **ID `0x100`** e **ID `0x123`**."
- Se chegar `0x100` ou `0x123`, passa. Se chegar `0x101`, o hardware rejeita.    
- _Desvantagem:_ Gasta muitos bancos se você precisar aceitar dezenas de IDs diferentes.


#### Modo Máscara (Identifier Mask Mode)
>Em vez de checar o ID inteiro, você usa dois valores: o **ID de Referência** e a **Máscara**. A Máscara diz ao hardware **quais bits do ID ele deve verificar** e quais bits ele deve **ignorar**.


- Bit na Máscara = **`1`** $\rightarrow$ **"Obrigatório!"** (O bit da mensagem recebida TEM que ser idêntico ao bit do ID de Referência).
- Bit na Máscara = **`0`** $\rightarrow$ **"Tanto faz!"** (Ignora esse bit, pode ser 0 ou 1 na mensagem que chega).


---
# Filas FIFO (FIFO 0 e FIFO 1)

Uma vez que a mensagem passa pelo filtro, ela precisa ser armazenada em algum lugar enquanto a CPU não vem buscá-la. Esse lugar é a **FIFO** (_First-In, First-Out_ / Fila de Chegada).

O periférico bxCAN do STM32 tem **duas filas físicas separadas**: a **FIFO 0** e a **FIFO 1**. Cada FIFO consegue guardar até 3 mensagens simultaneamente.
  

Na configuração do filtro, você escolhe para onde a mensagem aprovada vai:
```c
sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0; // Joga na FIFO 0
```

#### Por que existem duas FIFOs?
Para **gerenciamento de prioridades**. Você pode, por exemplo:

- Configurar o Banco 0 para jogar mensagens de **Emergência / Parada Crítica** na **FIFO 0** (com uma interrupção de altíssima prioridade no código).    
- Configurar o Banco 1 para jogar mensagens de **Sensores da máquina** na **FIFO 1** (com uma interrupção de menor prioridade)

---
# Escala do Filtro (16-bit vs 32-bit)

```c
sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
```

- **Modo 32-bit:** Reserva o banco inteiro de registradores para tratar **1 ID Estendido (29 bits)** ou 1 ID Standard com máscara completa.

- **Modo 16-bit:** Divide o banco ao meio, permitindo criar **2 regras de filtro de 16 bits** dentro de um único banco (ideal se você só usa o padrão CAN 2.0A com IDs de 11 bits e quer economizar bancos).

### Resumo visual do caminho da mensagem
$$\text{Mensagem no Cabo CAN} \longrightarrow \text{Banco de Filtro (Máscara/Lista)} \longrightarrow \text{FIFO 0 ou FIFO 1} \longrightarrow \text{Interrupção para o STM32}$$

---

