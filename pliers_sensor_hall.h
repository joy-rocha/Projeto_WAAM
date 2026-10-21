/*
 * sensor_corrente_rs485.h
 *
 * Leitura do sensor de corrente (alicate efeito Hall) via RS485/Modbus RTU.
 * MCU: STM32F407VET6  |  Periférico: USART2 (PA2 = TX, PA3 = RX)  |  Biblioteca: HAL
 *
 * O módulo TTL<->RS485 tem controle automático de direção (sem GPIO de DE/RE).
 * Tudo que costuma mudar está nos #defines abaixo.
 */
 
#ifndef SENSOR_CORRENTE_RS485_H
#define SENSOR_CORRENTE_RS485_H
 
#ifdef __cplusplus
extern "C" {
#endif
 
#include "main.h"
#include <stdint.h>
 
/* =====================================================================
 *  CONFIGURAÇÕES (altere aqui)
 * ===================================================================== */
 
/* ---- UARTs ---- */
#define SENSOR_UART             huart2      /* UART ligada ao módulo RS485   */
#define DEBUG_UART              huart1      /* UART ligada ao notebook       */
#define SENSOR_USART_INSTANCE   USART2
 
/* ---- Parâmetros seriais do sensor (8N1, 9600) ---- */
#define SENSOR_BAUDRATE         9600U
#define SENSOR_WORDLENGTH       UART_WORDLENGTH_8B
#define SENSOR_STOPBITS         UART_STOPBITS_1
#define SENSOR_PARITY           UART_PARITY_NONE
 
/* ---- Protocolo Modbus RTU ---- */
#define SENSOR_ADDR             0x01U       /* endereço do escravo           */
#define FUNC_READ_REGS          0x03U       /* função: ler registradores     */
#define REG_CORRENTE            0x000EU     /* registrador do valor medido   */
#define REG_QTD                 0x0001U     /* qtd de registradores a ler    */
 
/* ---- Tamanhos dos frames (derivados; REG_QTD = 1 -> resposta de 7 bytes) ---- */
#define REQ_LEN                 8U          /* addr+func+reg(2)+qtd(2)+crc(2)*/
#define RESP_DATA_BYTES         (2U * REG_QTD)
#define RESP_LEN                (3U + RESP_DATA_BYTES + 2U) /* addr+func+n+dados+crc */
 
/* ---- Posições dentro da resposta ---- */
#define RESP_IDX_ADDR           0U
#define RESP_IDX_FUNC           1U
#define RESP_IDX_NBYTES         2U
#define RESP_IDX_DATA_H         3U          /* byte alto do 1º registrador   */
#define RESP_IDX_DATA_L         4U          /* byte baixo do 1º registrador  */
#define RESP_IDX_CRC_L          (3U + RESP_DATA_BYTES)
#define RESP_IDX_CRC_H          (RESP_IDX_CRC_L + 1U)
 
/* ---- Timeouts (ms) ---- */
#define TX_TIMEOUT_MS           100U
#define RX_TIMEOUT_MS           500U
 
/* ---- CRC-16 Modbus ---- */
#define CRC_INIT                0xFFFFU
#define CRC_POLY                0xA001U
 
/* ---- Códigos de retorno (tipo HAL_StatusTypeDef) ---- */
#define SENSOR_OK               HAL_OK
#define SENSOR_ERR_FRAME        HAL_ERROR   /* endereço/função/tamanho errado */
#define SENSOR_ERR_CRC          HAL_ERROR   /* CRC da resposta não confere    */
 
/* =====================================================================
 *  API
 * ===================================================================== */
 
/* Inicializa a UART do sensor (9600 8N1).
 * Use só se o CubeMX NÃO gerou o MX_USART2_UART_Init() (evita duplicar). */
void sensor_uart_init(void);
 
/* Lê o registrador de corrente.
 * Retorna SENSOR_OK e preenche *valor_bruto em caso de sucesso.
 * A conversão para Ampères depende do fator de escala do manual. */
HAL_StatusTypeDef sensor_corrente_ler(uint16_t *valor_bruto);
 
#ifdef __cplusplus
}
#endif
 
#endif /* SENSOR_CORRENTE_RS485_H */
