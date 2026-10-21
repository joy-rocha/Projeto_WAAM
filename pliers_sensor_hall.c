/*
 * sensor_corrente_rs485.c
 *
 * Implementação da leitura do sensor de corrente via RS485/Modbus RTU.
 * Configurações e protótipos: ver sensor_corrente_rs485.h
 */
 
#include "sensor_corrente_rs485.h"
#include <stdio.h>
 
extern UART_HandleTypeDef DEBUG_UART;
extern UART_HandleTypeDef SENSOR_UART;
 
/* Redirecionamento do printf para a UART do notebook.
 * Remova se o projeto já tiver um _write (ex.: syscalls.c). */
int _write(int file, char *ptr, int len)
{
    (void)file;
    HAL_UART_Transmit(&DEBUG_UART, (uint8_t *)ptr, (uint16_t)len, HAL_MAX_DELAY);
    return len;
}
 
/* CRC-16 Modbus */
static uint16_t modbus_crc16(const uint8_t *data, uint16_t len)
{
    uint16_t crc = CRC_INIT;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t b = 0; b < 8; b++) {
            crc = (crc & 1U) ? (uint16_t)((crc >> 1) ^ CRC_POLY) : (uint16_t)(crc >> 1);
        }
    }
    return crc;
}
 
/* Monta a requisição: 01 03 00 0E 00 01 E5 C9 (com os valores padrão) */
static void sensor_monta_requisicao(uint8_t req[REQ_LEN])
{
    req[0] = SENSOR_ADDR;
    req[1] = FUNC_READ_REGS;
    req[2] = (uint8_t)(REG_CORRENTE >> 8);
    req[3] = (uint8_t)(REG_CORRENTE & 0xFFU);
    req[4] = (uint8_t)(REG_QTD >> 8);
    req[5] = (uint8_t)(REG_QTD & 0xFFU);
 
    uint16_t crc = modbus_crc16(req, REQ_LEN - 2U);
    req[6] = (uint8_t)(crc & 0xFFU);   /* CRC byte BAIXO primeiro */
    req[7] = (uint8_t)(crc >> 8);      /* CRC byte ALTO depois    */
}
 
HAL_StatusTypeDef sensor_corrente_ler(uint16_t *valor_bruto)
{
    uint8_t req[REQ_LEN];
    uint8_t resp[RESP_LEN] = {0};
    HAL_StatusTypeDef st;
 
    sensor_monta_requisicao(req);
 
    /* Descarta lixo no registrador de recepção */
    __HAL_UART_FLUSH_DRREGISTER(&SENSOR_UART);
 
    /* 1) Envia a requisição */
    st = HAL_UART_Transmit(&SENSOR_UART, req, REQ_LEN, TX_TIMEOUT_MS);
    if (st != HAL_OK) return st;
 
    /* 2) Recebe a resposta */
    st = HAL_UART_Receive(&SENSOR_UART, resp, RESP_LEN, RX_TIMEOUT_MS);
    if (st != HAL_OK) return st;
 
    /* 3) Valida endereço, função e quantidade de bytes de dados */
    if (resp[RESP_IDX_ADDR]   != SENSOR_ADDR     ||
        resp[RESP_IDX_FUNC]   != FUNC_READ_REGS  ||
        resp[RESP_IDX_NBYTES] != RESP_DATA_BYTES)
        return SENSOR_ERR_FRAME;
 
    /* 4) Valida CRC (na resposta: byte baixo, depois byte alto) */
    uint16_t crc_calc = modbus_crc16(resp, RESP_IDX_CRC_L);
    uint16_t crc_recv = (uint16_t)resp[RESP_IDX_CRC_L] |
                        ((uint16_t)resp[RESP_IDX_CRC_H] << 8);
    if (crc_calc != crc_recv)
        return SENSOR_ERR_CRC;
 
    /* 5) Valor medido: byte alto + byte baixo (ex.: 05 DC -> 1500) */
    *valor_bruto = ((uint16_t)resp[RESP_IDX_DATA_H] << 8) | resp[RESP_IDX_DATA_L];
    return SENSOR_OK;
}
 