
---
### ***Seleção do Protocolo e Microcontrolador***

 **Limitação do Hardware:** O microcontrolador **STM32F412RET6** possui periféricos **bxCAN** (Basic Extended CAN), que suportam nativamente o padrão **CAN 2.0A** (ID Standard de 11 bits) e **CAN 2.0B** (ID Extended de 29 bits), **não possuindo suporte a CAN FD** (Flexible Data-rate).
 
**Decisão de Projeto:** Optou-se pelo uso da norma **CAN 2.0A (Standard - 11 bits)** rodando a uma taxa de transmissão de **500 kbps**, o que fornece até 2.048 identificadores únicos — capacidade mais do que suficiente para o controle e monitoramento do sistema.

---
### ***Imunidade Eletromagnética no Ambiente de Soldagem***

**Sinalização Diferencial ($CAN\_H$ / $CAN\_L$):** O ruído gerado pelo arco elétrico e correntes elevadas de soldagem atinge os dois fios da rede simultaneamente (ruído em modo comum). Como o protocolo lê apenas a **diferença de tensão** entre os fios, a interferência é cancelada eletronicamente.

**Isolamento Galvânico (WeAct ISOCANFD V1):** O transceiver utilizado possui isolamento elétrico integrado entre a lógica de 3,3V do STM32 e a linha física do barramento. Isso protege o microcontrolador contra surtos de tensão, laços de terra (_ground loops_) e descargas do ambiente de solda.

 **Resistores de Terminação:** Utilização dos resistores de **120 $\Omega$** nas extremidades da linha para evitar reflexão de onda e atenuação do sinal.

---
### ***Hardware da Bancada de Teste e Validação***

**Unidade de Processamento:** STM32F446RET6
(executando firmware em C configurado via STM32CubeMX + HAL).
  
**Interface Transceptora:** Módulo WeAct Studio ISOCANFD V1 
(conversão TTL/Diferencial com isolamento galvânico).

 **Analisador de Barramento:** Adaptador USB2CANFD V2
 (ponte de comunicação entre o barramento físico CAN e o computador PC via USB).

---
### ***Estratégia de Filtragem e Priorização por Hardware (Dual FIFO)***

- **Filtro de Alta Prioridade (Banco 0 $\rightarrow$ FIFO 1):** Mapeado para capturar exclusivamente mensagens de emergência ou parada de segurança (faixa de IDs `0x010` a `0x01F`). Essas mensagens são direcionadas à **FIFO 1**, acionando uma rotina de interrupção com prioridade máxima no NVIC.

- **Filtro de Tráfego Geral (Banco 1 $\rightarrow$ FIFO 0):** Configurado em modo máscara zerada ("passa-tudo") para direcionar o restante da telemetria e comandos genéricos para a **FIFO 0**.

---
