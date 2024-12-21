/*
 * UART.h
 *
 *  Created on: Dec 21, 2024
 *      Author: sh124
 */
#ifndef UART_H
#define UART_H

#include "MKE16Z4.h"
#define UART_TIMEOUT 0xFFFFFFFF // Giá trị timeout

typedef enum
{
    UART_SUCCESS = 0,
    UART_ERROR = 1,
    UART_ERROR_TIMEOUT = 3,
} uart_status_t;

typedef struct
{
    uint32_t baudRate;
    uint32_t clockSrc;
} uart_config_t;

void UART_Init(LPUART_Type *base, uart_config_t *config);
uart_status_t UART_WriteByte(LPUART_Type *base, uint8_t data);
uint8_t UART_ReadByte(LPUART_Type *base);
void UART_EnableInterrupts(LPUART_Type *base);
void UART_DisableInterrupts(LPUART_Type *base);

#endif // UART_H


