
O **PWM** (_Pulse Width Modulation_ ou Modulação por Largura de Pulso) é uma técnica em que o microcontrolador gera um sinal quadrado alternando rapidamente entre 3.3V (nível ALTO) e 0V (nível BAIXO). Alterando a proporção de tempo em que o sinal fica ligado (_Duty Cycle_), controla-se a tensão média entregue ao motor e, consequentemente, sua velocidade.

  
### 1. A Lógica do Hardware: Como o Timer do STM32 Gera PWM

Internamente, os Timers do STM32 funcionam como contadores digitais baseados no relógio do sistema (_Clock_). Três parâmetros principais controlam o PWM:

1. **Prescaler (PSC):** Divisor do clock do microcontrolador. Ele reduz a frequência do clock principal antes de enviar os pulsos para o contador.
    
2. **Auto Reload Register (ARR ou Counter Period):** É o valor máximo que o contador alcança antes de zerar. Ele define o **período** (e a **frequência**) da onda PWM.
    
3. **Capture Compare Register (CCR ou Pulse):** É o valor de comparação que define o **Duty Cycle**.
  

#### Como funciona o ciclo de contagem:
- O contador começa em `0` e vai subindo até o valor de **ARR**.
- Enquanto o valor do contador for **menor que CCR**, a saída do pino permanece em **3.3V** (HIGH).
- Quando o contador **ultrapassa CCR**, a saída muda para **0V** (LOW).
- Quando o contador atinge **ARR**, ele reseta para `0` e o ciclo recomeça.

#### Fórmulas de Cálculo:

$$\text{Frequência do PWM } (f_{PWM}) = \frac{f_{CLK}}{(PSC + 1) \cdot (ARR + 1)}$$

$$\text{Duty Cycle (\%)} = \left( \frac{\text{CCR}}{ARR + 1} \right) \cdot 100$$

---

### 2. Configuração Passo a Passo no STM32CubeMX

No esquema elétrico, os pinos definidos para a ponte H são **PB6 (`PH_RPWM`)** e **PB7 (`PH_LPWM`)**. No STM32F412, esses pinos são mapeados para o periférico **TIM4** (Canais 1 e 2).

  

#### Passo 1: Mapear os Pinos

1. Abra a aba **Pinout & Configuration** no CubeMX.
    
      
    
2. No circuito integrado gráfico, localize o pino **PB6**, clique nele e selecione **`TIM4_CH1`**.
    
      
    
3. Localize o pino **PB7**, clique nele e selecione **`TIM4_CH2`**.
    
      
    

#### Passo 2: Ativar os Canais do Timer

1. No menu esquerdo, vá em **Categories** $\rightarrow$ **Timers** $\rightarrow$ **TIM4**.
    
      
    
2. Na configuração do **TIM4**:
    
      
    - **Channel1:** Selecione **PWM Generation CH1**.
        
          
        
    - **Channel2:** Selecione **PWM Generation CH2**.
        
          
        

#### Passo 3: Configurar os Parâmetros (Exemplo Prático)

Motores DC operam bem com frequências de PWM entre **10 kHz e 20 kHz** (frequências acima de 16 kHz evitam ruídos audíveis de apito no motor).

  

Supondo que o Clock do timer (`f_CLK`) esteja operando a **84 MHz**:

  

1. Na aba **Parameter Settings** do **TIM4**:
    
      
    - **Prescaler (PSC):** `0` _(Sinal sem divisão primária)_.
        
          
        
    - **Counter Period (ARR):** `4199` _(Define a frequência em 20 kHz)_.
        
          
        
    - **Counter Mode:** Up.
        
          
        
2. Na seção **PWM Generation Channel 1** e **Channel 2**:
    
      
    - **Pulse:** `0` _(Inicia com velocidade zerada)_.
        
          
        
    - **PWM Mode:** Mode 1.
        
          
        

> **Verificação da Frequência:**
> 
>   
> 
> $$f_{PWM} = \frac{84.000.000}{(0 + 1) \cdot (4199 + 1)} = \frac{84.000.000}{4200} = 20.000\text{ Hz } (20\text{ kHz})$$
> 
> Com `ARR = 4199`, a resolução do motor será de **0 a 4200 passos** (sendo 0 = 0% e 4200 = 100% de velocidade).
> 
>   



---
### 3. Como Isso é Aplicado no Código (Interface HAL)

Após gerar o código no CubeMX, o controle do motor é feito em duas etapas simples na função `main()`:

  

#### Inicializar a geração de PWM:

C

```
// Ativa o PWM nos dois canais do Timer 4
HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1); // PB6 (RPWM)
HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2); // PB7 (LPWM)
```

#### Alterar a velocidade e sentido durante a execução:

- **Para girar para a Direita a 50% de velocidade:**
    
      
    
    C
    
    ```
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 2100); // 50% de 4200 no RPWM
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);    // 0% no LPWM
    ```
    
- **Para girar para a Esquerda a 80% de velocidade:**
    
      
    
    C
    
    ```
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);    // 0% no RPWM
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 3360); // 80% de 4200 no LPWM
    ```
    
- **Para parar o motor:**
    
    C
    
    ```
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);
    ```
