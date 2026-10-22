
---
O **CAN** (_Controller Area Network_) é um protocolo serial veicular e industrial onde um único barramento partilhado interliga todas as unidades de comando. Esse barramento possui os fios `CAN HIGH` e `CAN LOW` que são entrançados para anular interferências eletromagnéticas. Assim, quando um ruído externo afeta ambos os fios na mesma proporção, mantendo a diferença de tensão intacta. Seu funcionamento é baseado na diferença de tenção entre os fios, chamada **Medição Diferencial**. O receptor calcula a tensão através da fórmula:
$$V_{\text{diferencial}} = V_{\text{CAN\_H}} - V_{\text{CAN\_L}}$$

 Com base no valor do cálculo ele:
- **Bit 1 (Estado Recessivo):** Quando ambos os fios ficam na mesma faixa de tensão (diferença próxima de $0\text{V}$), o barramento fica em **estado recessivo** e o circuito lê o bit **1**.

- **Bit 0 (Estado Dominante):** Quando há uma diferença clara de tensão entre os dois fios (aproximadamente $2,5\text{V}$), o barramento entra em **estado dominante** e o circuito lê o bit **0**.

---
## LÓGICA DE COMO USAR O CAN 

***==🟡IMPORTAÇÃO e REQUISITOS==***
 Importe o CAN, habilite as interrupçẽos (ele sinaliza quando uma mensagem passa pelo filtro definido) e defina o Baud Rate (Você ajusta a taxa de transmissão da rede) e o software já separa automaticamente o dois pinos para o TX e o RX. 

o pino TX manda sinais digitais que são convertidos pelo Transceiver para analógico, gerando as tensões certas no HIGH e no LOW, o RX monitora o CAN_H e o CAN_L, calcula a diferença e transforma em um sinal digital (0 ou 1) enviado pelo RX.

==***CONFIGURAÇÃO DO FILTRO***==
É necessário a configuração de um filtro, que  funciona totalmente via hardware, ele verifica o id da mensagem e se não for do interesse do chip ele já descarta, se for um id válido ele da passagem para o uso da CPU. (o STM32F446RET6 possui 28 bancos de filtros no total).

***==PADRÃO DE MENSAGEM==***
como só tem um barramento para toda a comunicação o CAN possui uma espécie de id que indica o assunto da mensagem (título, categoria, conteúdo da mensagem) 

- **Exemplo:** O ID `0x100` significa _"Velocidade Atual do Veículo"_. O módulo do motor apenas "grita" no barramento: `ID 0x100: 80 km/h`. Quem precisa dessa informação (como o painel e o freio ABS) aceita esse ID no seu filtro. Quem não precisa (como o ar-condicionado) ignora.

==***ENVIO e RECEBIMENTO de mensagens***==
Para o envio de mensagem é necessário preencher a header, nela definimos o id, o tamanho do dados (0 a 8 bytes) e o formato, e os dados são passados num vetor e chama a função de envio.

Já na recepção quando chega uma dado no barramento o callback é chamado e ela já extrai a mensagem pra receber o id e o dado

```
[ Hardware Transceiver ] -> Cuida dos níveis elétricos dos fios CAN_H / CAN_L
        │
[ Pinos TX / RX ]       -> Ligados ao STM32
        │
[ Filtro da HAL ]       -> Deixa passar apenas o ID configurado
        │
[ Interrupção / FIFO ]  -> Dispara o Callback automaticamente
        │
[ Seu Código C ]        -> Lê o vetor de dados e processa a informação
```

---
### ***CAN NA PRÁTICA***

*Sequência rigorosa de configuração no C!*

`HAL_CAN_ConfigFilter()` $\rightarrow$ Configura as regras do filtro primeiro.
`HAL_CAN_Start()` $\rightarrow$ Liga o periférico CAN no chip.
`HAL_CAN_ActivateNotification()` $\rightarrow$ Ativa os avisos por interrupção.

---
## ***configurando o CAN no MX***
em connectivity ative o CAN1, depois clique na aba NVIC Settings, marque a caixinha ao lado de CAN1 RX0 interrupt. em Parameters Settings, vá em Bit Timings Parameters ->Para configurar a rede para a velocidade padrão de **500 kbps**, altera os seguintes campos: Prescaler: `2`, Time Segment 1: `13 Times`, Time Segment 2: `2 Times` -> GENERATE COD

![[Pasted image 20261019130728.png|212]]


---
# CAN Frame

Organização é o layout oficial da norma ISO 11898-1 que define como os bits são agrupados na camada de enlace antes de serem enviados no barramento físico.

![[Pasted image 20261020104858.png]]

$-$ O tamanho máximo da mensagem completa (**overhead/header + dados**), considerando o pior cenário (carga útil máxima e formato estendido), ==chega a **128 bits no CAN Clássico**==

**Vamos utilizar o CAN 2.0A (Standard - 11 bits)**
```c
TxHeader.IDE = CAN_ID_STD;  // Define ID Padrão (11 bits - CAN 2.0A)
TxHeader.StdId = 0x100;     // ID curto (3 dígitos hexadecimais)
```

Nesse modo do CAN é **preciso definir apenas 5 parâmetros no código C**; o chip e a biblioteca HAL geram e injetam todo o restante automaticamente no barramento físico.

| **Campo no Quadro CAN**         | **Preenchimento** | **O que você faz no código C / Papel do Hardware**                                                 |
| ------------------------------- | ----------------- | -------------------------------------------------------------------------------------------------- |
| **SOF** _(Start of Frame)_      | **Automático**    | O hardware gera o bit dominante (`0`) para avisar à rede que a transmissão começou.                |
| **Message ID** (11 bits)        | **Manual**        | Você define na variável **`TxHeader.StdId`** (ex: `0x100` ou `0x123`).                             |
| **RTR** _(Request)_             | **Manual**        | Você define na variável **`TxHeader.RTR = CAN_RTR_DATA`** para indicar envio de dados.             |
| **IDE** _(Identifier Ext.)_     | **Manual**        | Você define na variável **`TxHeader.IDE = CAN_ID_STD`** para indicar modo CAN 2.0A (11 bits).      |
| **r0** _(Bit Reservado)_        | **Automático**    | O hardware força automaticamente o nível lógico dominante (`0`).                                   |
| **Data Length** _(DLC)_         | **Manual**        | Você define na variável **`TxHeader.DLC`** a quantidade de bytes a enviar (de `0` a `8`).          |
| **Data** _(0 a 8 bytes)_        | **Manual**        | Você preenche o vetor **`TxData[]`** com as informações reais que quer mandar.                     |
| **CRC** _(Checksum)_            | **Automático**    | O hardware calcula a equação matemática de verificação sobre todos os bits antes do envio.         |
| **ACK** _(Confirmação)_         | **Automático**    | O hardware envia o bit recessivo e escuta se outro nó no cabo puxou para dominante em confirmação. |
| **EOF / IFS** _(Fim do Quadro)_ | **Automático**    | O hardware injeta os bits de parada e respeita o tempo de silêncio mínimo do barramento.           |
| **Bit Stuffing**                | **Automático**    | O hardware insere um bit oposto a cada 5 bits repetidos para manter a sincronia de clock.          |

---
# Parâmetros manuais do frame CAN

### 1. Message ID (`TxHeader.StdId`) — O Identificador da Mensagem

>Um número hexadecimal de 11 bits (vai de `0x000` até `0x7FF`) que identifica **o conteúdo do pacote**

- **Prioridade (Arbitragem):** Se duas placas tentarem transmitir ao mesmo tempo, **vence a mensagem que tiver o menor ID numérico**. O bit `0` (dominante) esmaga o bit `1` (recessivo). Portanto, um pacote de emergência/freio deve ter `ID = 0x001`, enquanto uma leitura de temperatura do ambiente pode ter `ID = 0x500`.


### 2. RTR (`TxHeader.RTR`) — Remote Transmission Request

>Um bit que define a intenção do pacote: **Envio de Dados** ou **Pedido de Dados**.

- **`CAN_RTR_DATA` (Bit = 0 / Dominante):** Significa: _"Estou enviando dados no campo Data."_ (É o modo usado em 99% das aplicações).

- **`CAN_RTR_REMOTE` (Bit = 1 / Recessivo):** Significa: _"Não estou mandando dados, estou perguntando. Placa X, por favor envie os dados desse ID."_ (Raramente usado na prática moderna, pois a maioria dos sistemas prefere trabalhar com envio periódico automático).


### 3. IDE (`TxHeader.IDE`) — Identifier Extension

>O bit seletor do formato da versão do protocolo CAN (2.0A ou 2.0B). Ele avisa aos receptores na rede qual o tamanho da estrutura do ID que está sendo transmitido naquele instante.

- **`CAN_ID_STD` (Bit = 0 / Dominante):** Informa à rede que é um quadro **CAN 2.0A** e que o ID termina nos 11 bits. O hardware do receptor sabe que o próximo bit após o ID já é o controle do tamanho de dados.

- **`CAN_ID_EXT` (Bit = 1 / Recessivo):** Informa à rede que é um quadro **CAN 2.0B** e que há mais 18 bits de ID chegando na sequência (totalizando 29 bits).


### 4. DLC (`TxHeader.DLC`) — Data Length Code

>Um número de 4 bits que define a quantidade de bytes no campo de dados (vai de `0` a `8`). Informa à placa receptora quantos bytes ela deve ler no campo de carga útil.

**Como funciona:**
- Se você quer mandar apenas um comando "Ligar/Desligar", pode definir `DLC = 1` (economiza tempo de barramento e diminui o tamanho do pacote).
- Se precisa enviar múltiplos sensores de uma vez, define `DLC = 8`.
- O receptor lê esse valor e sabe exatamente a partir de qual bit a carga de dados termina e começa a checagem de erros (CRC).

### 5. Data Field (`TxData[]`) — A Carga Útil (Payload)

> É a informação real que você deseja trafegar entre o computador e o microcontrolador. É uma matriz (array) de bytes na memória que guarda até 8 valores de 8 bits cada (`TxData[0]` a `TxData[7]`).

**Como funciona:** 
Você pode fatiar esses 8 bytes da forma que achar melhor para a arquitetura do seu projeto:
  
- **Byte 0 (`TxData[0]`):** Comando (ex: `0x01` = Ligar Motor, `0x02` = Desligar Motor).
    
- **Bytes 1 e 2 (`TxData[1]` e `TxData[2]`):** Leitura de um sensor de 16 bits (ex: Velocidade do Motor = 1500 RPM).
    
- **Byte 3 (`TxData[3]`):** Temperatura em graus Celsius.
    
- **Bytes 4 a 7:** Reservados para códigos de erro ou diagnóstico.



---
# Dependências importantes para o bom funcionamento do CAN

### Resistores de Terminação (120 $\Omega$)
>O barramento CAN exige **um resistor de 120 $\Omega$ em cada extremidade física do cabo** (ligado entre os fios $CAN\_H$ e $CAN\_L$). Sem esses dois resistores nas pontas, o sinal elétrico "reflete" nos fios como um eco, corrompendo os bits e gerando erros de CRC.

 - (O módulo WeAct ISOCANFD e o analisador USB2CAN geralmente possuem jumpers ou chaves seletoras (escrito **120R**) para ativar esses resistores no próprio circuito)

### A Regra do ACK 
> Se você ligar apenas o STM32 na bancada e mandar transmitir uma mensagem sem ter **nenhum outro dispositivo ligado ao cabo**, o STM32 vai dar erro e travar a transmissão.
-
 >No protocolo CAN, quem envia o pacote exige que pelo menos um receptor na rede puxe o bit de **ACK** para nível dominante (`0`) para confirmar a leitura. Se ninguém der o ACK, o transmissor assume erro de linha e fica tentando retransmitir infinitamente.

# Baud Rate
>Todos os nós da rede **precisam estar programados exatamente com a mesma velocidade**
  (no projeto configuramos **500 kbps**).
  
- Se o STM32 estiver a 500 kbps e o software do PC estiver configurado a 250 kbps, o conversor USB vai interpretar os bits no tempo errado e jogar quadros de erro (_Error Frames_) no barramento.

# Auto-Isolamento de Segurança (Estado de _Bus-Off_)
> O periférico CAN do STM32 tem contadores internos de erro de transmissão e recepção (TEC e REC). Se o cabo estiver mau conectado ou houver ruído excessivo, o controlador entra no estado **Bus-Off** e se desconecta sozinho da rede por segurança. Se isso acontecer, o código precisa reiniciar o periférico (`HAL_CAN_Start`) para voltar a falar.


---


