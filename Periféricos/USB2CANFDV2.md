
| ![[Pasted image 20261019130844.png\|227]] |
| ----------------------------------------- |
[documentação_USB2CANFDV2](https://github.com/WeActStudio/WeActStudio.USB2CANFDV2)

---
### Para que serve o software do USB2CANFD V2?

O conversor USB2CANFD V2 é uma "ponte" entre os cabos físicos da rede e o seu computador. Você precisa de um software no PC para três coisas principais:

1. **Configurar o parâmetro de rede (Baud Rate):** Dizer ao analisador USB em qual velocidade o barramento físico está operando (no nosso caso, **500 kbps**). Se você não avisar a velocidade ao conversor, ele não consegue sincronizar a leitura dos bits nos cabos.

2. **Visualizar o que o STM32 envia (Sniffer/Monitor):** Mostrar na tela do PC os pacotes em tempo real quando o STM32 transmitir (exibir o ID `0x100`, o tamanho DLC `8` e os bytes de dados).

3. **Enviar comandos de teste:** Digitar um ID (ex: `0x123`) e enviar dados do PC em direção ao STM32 para testar se a interrupção de recepção do STM32 está funcionando.

---


**Cangaroo (Interface Gráfica Gratuita):** [documentação_cangaroo](https://github.com/WeActStudio/cangaroo)
>É o software recomendado pela fabricante (WeAct)

- **Isolamento de 2.500V na ponta do USB:** O chip conversor (`CA-IS2062A`) do analisador USB já possui **isolação galvânica interna de 2,5 kV**. Somado ao módulo isolado WeAct que vai no STM32, você tem dupla proteção contra o ruído da máquina de solda!
    
- **O programa Cangaroo já está dentro do repositório:** Você não precisa ficar procurando na internet. Se baixar o repositório em `.zip`, o Cangaroo estará pronto na pasta **`Tools/cangaroo`**
    
- **Firmware SLCAN de fábrica:** Ele vem gravado por padrão com o firmware **SLCAN**, que cria uma porta COM virtual no Windows. É exatamente por isso que o nosso código Python com `interface='slcan'` e o Cangaroo vão funcionar sem precisar mexer em nada na placa.
    
- **Comando de velocidade de 500 kbps (`S6`):** O protocolo SLCAN usa comandos de texto na porta serial para configurar a velocidade. O comando para os **500 kbps** que configuramos no STM32 é o **`S6`** (o Cangaroo ou a biblioteca `python-can` enviam isso automaticamente para você).

---
### Como usar o Cangaroo na prática quando a bancada estiver pronta

1. **Conexão:** Pluga o **USB2CANFD V2** no PC via USB. Ele vai criar uma porta COM virtual no Windows.

2. **Abertura da Interface:**
	- Abre o Cangaroo.
	- Seleciona o driver de comunicação (**SLCAN** ou **SerialPort**).
	- Escolhe a porta COM atribuída ao analisador.
	- Configura a velocidade para **500 kbps** (a mesma configurada no STM32).

3. **Início do Teste:** Clica em **Start / Connect**.
---
### O que acontece durante a validação

- **Leitura (STM32 $\rightarrow$ PC):** Quando o STM32 ligar e começar a rodar o loop, o Cangaroo vai listar em tempo real a mensagem transmitida a cada 1 segundo (ID `0x100`, DLC `8`, dados `01 02 03 04 05 06 07 08`).

- **Envio (PC $\rightarrow$ STM32):** No campo de transmissão do Cangaroo, podes digitar o ID `0x123` com alguns bytes de dados e clicar em **Send**. A interrupção de recepção (`HAL_CAN_RxFifo0MsgPendingCallback`) do STM32 vai disparar imediatamente no microcontrolador.


---
# ==🟢README.MD==
## Descrição do Hardware
- **Microcontrolador:** STM32G431CBT6 (170 MHz Máx, 32 KB RAM, 128 KB Flash).
- **CANFD / CAN 2.0:** Isolação de 2500V, até 5 Mbps. Suporta o Cangaroo ou desenvolvimento próprio. Tranceiver: CA-IS2062A (Chipanalog).
- **Conector:** Borne KRE de 3,81 mm (3 pinos).

## Descrição dos Firmwares
- **Firmware SLCAN (USB CDC):** Suporta o software Cangaroo (é o firmware padrão de fábrica).
- **Firmware Candlelight2.5 (gs_usb):** Suporta o software HUD-ECU-Hacker.

## Comandos do Protocolo SLCAN (Via Serial/USB)

> _Atualize para o firmware mais recente para suporte a mais taxas de transmissão._

- **Controle da Rede:**
    
    - `O`: Abre o canal CAN.
        
    - `C`: Fecha o canal CAN.
        
    - `M0`: Modo Normal (Padrão).
        
    - `M1`: Modo Silencioso (Apenas escuta a rede, sem enviar ACK ou quadros).
        
    - `A0`: Desativa retransmissão automática (Padrão).
        
    - `A1`: Ativa retransmissão automática (Não recomendado, pode travar).
        
- **Configuração de Velocidade (Baud Rate Nominal):**
    
    - `S0`: 10 kbps | `S1`: 20 kbps | `S2`: 50 kbps | `S3`: 100 kbps
        
    - `S4`: 125 kbps (Padrão) | `S5`: 250 kbps
        
    - **`S6`: 500 kbps** _(A velocidade do nosso projeto)_
        
    - `S7`: 800 kbps | `S8`: 1 Mbps
        
- **Envio de Mensagens CAN 2.0:**
    
    - `tIIILDD...`: Envia quadro de dados com **ID Padrão (11 bits)** [ID, Tamanho, Dados].
        
    - `TIIIIIIIILDD...`: Envia quadro de dados com **ID Estendido (29 bits)** [ID, Tamanho, Dados].
        
    - `rIIIL`: Envia quadro remoto (RTR) com ID Padrão.
        
    - `RIIIIIIIIL`: Envia quadro remoto (RTR) com ID Estendido.
        
- **Funções de Diagnóstico e Filtro:**
    
    - `V`: Lê a versão do firmware do analisador.
        
    - `E`: Lê o estado de falhas/erros da rede.
        
    - `X`: Entra no modo de atualização de firmware.
        
    - `fIIIMMM`: Configura filtro para ID padrão [ID, Máscara].
        

## Descrição dos LEDs e Pinos da Placa

- **PA0 (`LED_RXD`):** LED indicador de recepção (pisca quando chegam dados da rede).
- **PA1 (`LED_TXD`):** LED indicador de transmissão (pisca quando o PC envia dados).
- **PA2 (`LED_READY`):** LED de status (pisca a cada 0,5s quando a porta CAN é aberta no PC).

## Como Gravar/Atualizar o Firmware

1. Pressione e segure o botão **`BOOT0`** no analisador e conecte o cabo USB no PC.
2. Abra o software **STM32CubeProgrammer** e selecione a interface USB.
3. Importe o arquivo de firmware (`.bin` ou `.hex`) e clique em **Download**.
4. Desconecte e reconecte a placa após concluir.

---