/*
 * inputs.c
 *
 *  Created on: Apr 30, 2025
 *      Author: rolf
 */


#include "inputs.h"

void Inputs_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();  // Enable GPIOA

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Initialize Vann (PA8)
    GPIO_InitStruct.Pin = Vann_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL; // or GPIO_PULLUP depending on hardware
    HAL_GPIO_Init(Vann_GPIO_PORT, &GPIO_InitStruct);

    // Initialize AlertIO (PA4)
    GPIO_InitStruct.Pin = AlertIO_PIN;
    HAL_GPIO_Init(AlertIO_GPIO_PORT, &GPIO_InitStruct);

    // Initialize OverTMP (PA5)
    GPIO_InitStruct.Pin = OverTMP_PIN;
    HAL_GPIO_Init(OverTMP_GPIO_PORT, &GPIO_InitStruct);

    // Initialize CritTMP (PA6)
    GPIO_InitStruct.Pin = CritTMP_PIN;
    HAL_GPIO_Init(CritTMP_GPIO_PORT, &GPIO_InitStruct);
}

uint8_t Power_meas_Alert(void) {
	return HAL_GPIO_ReadPin(AlertIO_GPIO_PORT, AlertIO_PIN) == GPIO_PIN_RESET; // active-low
}
//uint8_t (void) {
//	return HAL_GPIO_ReadPin(,) == GPIO_PIN_RESET; // active-low
//}
