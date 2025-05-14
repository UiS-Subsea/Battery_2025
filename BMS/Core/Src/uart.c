/*
 * uart.c
 *
 *  Created on: Apr 30, 2025
 *      Author: rolf
 */


#include "uart.h"
#include <stdarg.h>   // needed for va_list and related macros
#include <stdio.h>    // needed for vsnprintf
#include <string.h>   // for strlen

void UART_printf(const char *format, ...) {
    char buffer[128];  // adjust size as needed
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    UART_send((uint8_t *)buffer, strlen(buffer));
}
void UART_send(uint8_t *data, uint16_t size) {
    HAL_UART_Transmit(&huart2, data, size, HAL_MAX_DELAY);
}

void UART_send_float(float *data, uint16_t count) {
    HAL_UART_Transmit(&huart2, (uint8_t *)data, count * sizeof(float), HAL_MAX_DELAY);
}

void UART_send_str(const char *str) {
    HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

HAL_StatusTypeDef UART_receive(uint8_t *buffer, uint16_t size) {
    return HAL_UART_Receive(&huart2, buffer, size, HAL_MAX_DELAY);
}
