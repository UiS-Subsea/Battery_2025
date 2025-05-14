/*
 * uart.h
 *
 *  Created on: Apr 30, 2025
 *      Author: rolf
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "stm32g4xx_hal.h"
#include <stdint.h>

// External UART handle (defined in main.c or usart.c)
extern UART_HandleTypeDef huart2;

// Functions
void UART_printf(const char *format, ...);
void UART_send(uint8_t *data, uint16_t size);
void UART_send_float(float *data, uint16_t count);
void UART_send_str(const char *str);
HAL_StatusTypeDef UART_receive(uint8_t *buffer, uint16_t size);


#endif /* INC_UART_H_ */
