
| ![[Pasted image 20261021103531.png\|192]] |
| ----------------------------------------- |

---

O módulo **WeAct Studio ISOCANFD V1** é dividido em duas partes isoladas galvanicamente pelo chip **CA-IS2062A**: o **lado do microcontrolador (lógica)** e o **lado do barramento (linha CAN)**.

### 1. Lado da Lógica (Barra de Pinos Amarela — Conecta ao STM32)
Esta barra de pinos conecta-se diretamente à sua placa STM32F446RET6:

|**Nome no Módulo**|**Função do Pino**|**Onde conectar na placa STM32**|
|---|---|---|
|**VIO**|Tensão de referência lógica (2,5V a 5,5V)|Pino **3.3V** da STM32|
|**5V**|Alimentação do conversor DC-DC isolado interno|Pino **5V** (ou `5V_USB`) da STM32|
|**GND**|Terra lógico do microcontrolador|Pino **GND** da STM32|
|**TX** / **TXD**|Entrada do sinal de transmissão CAN|Pino **CAN1_TX** da STM32 (**PA12** ou **PB9**)|
|**RX** / **RXD**|Saída do sinal de recepção CAN|Pino **CAN1_RX** da STM32 (**PA11** ou **PB8**)|
|**S** / **SILENT** _(se presente)_|Modo silencioso (se em nível alto, desativa o TX)|Deixar **desconectado** ou ligado ao **GND**|

> **Nota sobre o pino 5V:** O chip CA-IS2062A possui um gerador DC-DC interno isolado que precisa dos **5V** da STM32 para alimentar o lado do barramento sem utilizar contato elétrico direto.

### 2. Lado da Rede (Borne Verde de Parafuso — Conecta ao USB2CANFD V2)
Estes bornes parafusáveis conectam-se à linha física do cabo que vai para o computador:

|**Nome no Borne**|**Função**|**Onde conectar no USB2CANFD V2**|
|---|---|---|
|**CAN_H**|Linha diferencial de alta velocidade|Borne **CAN_H** do USB2CANFD V2|
|**CAN_L**|Linha diferencial de baixa velocidade|Borne **CAN_L** do USB2CANFD V2|
|**GND_ISO** / **GND**|Terra isolado da rede CAN|Borne **GND** do USB2CANFD V2 _(opcional)_|

### 3. Conector / Jumper do Resistor de Terminação (`120R`)
No PCB há um par de pinos marcado como **120R** (ou um jumper verde/plástico):

- **Como usar:** Encaixe o jumper de plástico nos dois pinos para **ativar o resistor de $120\,\Omega$**.
    
- **Por que usar:** Em testes de bancada de curta distância entre o STM32 e o PC, o resistor de terminação deve ficar **ativado** nas duas pontas para evitar reflexão de sinal.

---
### Regra de Segurança Importante

- **NUNCA conecte o `GND_ISO` (borne verde) ao `GND` do STM32 (barra amarela).**

- O objetivo do módulo é criar uma **barreira de isolamento de 2,5 kV** entre a eletrônica sensível e o ambiente ruidoso de solda. Unir os dois terras anularia essa proteção.

---

**Perfeito! O seu esquema no diagrama ficou 100% correto!**

  

As conexões direto com direto ($H \to H$, $L \to L$, $TX \to TX$ e $RX \to RX$) e as alimentações separadas garantem que o isolamento galvânico funcione sem queimar o microcontrolador.

  

Aqui está a tabela completa com todas as conexões para você salvar no seu relatório ou no Obsidian:

  

### Tabela de Mapeamento de Conexões do Sistema CAN

#### 1. Lado da Linha Física (Barramento Diferencial)

|**Analisador USB2CANFD V2**|**Módulo ISOCANFD V1**|**Função no Cabo**|
|---|---|---|
|**H (CAN_H)**|**H (CAN_H)**|Sinal Diferencial Alto (Ligação Direta)|
|**L (CAN_L)**|**L (CAN_L)**|Sinal Diferencial Baixo (Ligação Direta)|
|**GND**|**G1 (GND_ISO)**|Terra do Barramento Isolado _(Opcional)_|

#### 2. Lado Lógico (Sinais Digitais e Alimentação)

|**Módulo ISOCANFD V1**|**Placa STM32F446RET6**|**Tipo de Sinal / Descrição**|
|---|---|---|
|**5V**|**5V** (ou `5V_USB`)|Alimentação do gerador isolado interno (5V)|
|**VIO**|**3V3**|Referência dos níveis lógicos TTL (3,3V)|
|**GND**|**GND**|Terra Lógico da CPU / Microcontrolador|
|**TX**|**PA12** (`CAN1_TX`)|Transmissão de Dados (Ligação Direta: TX $\to$ TX)|
|**RX**|**PA11** (`CAN1_RX`)|Recepção de Dados (Ligação Direta: RX $\to$ RX)|

> **Lembrete de Bancada:** Não se esqueça de fechar o _jumper_ escrito **120R** no módulo ISOCANFD V1 e ativar o resistor de terminação de $120\,\Omega$ no USB2CANFD V2 durante os testes de bancada!