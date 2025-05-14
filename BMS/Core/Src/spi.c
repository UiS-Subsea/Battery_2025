/*
 * spi.c
 *
 *  Created on: Apr 30, 2025
 *      Author: rolf
 */


#include "spi.h"
#include "functions.h"

void SPI_Devices_Init(void) {
//    __HAL_RCC_GPIOA_CLK_ENABLE();  // Enable GPIOA clock

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Init CS for Device 1 (PA9)
    GPIO_InitStruct.Pin = CS_Overvoker_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(CS_Overvoker_GPIO_PORT, &GPIO_InitStruct);
    // Deselect device initially
    HAL_GPIO_WritePin(CS_Overvoker_GPIO_PORT, CS_Overvoker_PIN, GPIO_PIN_SET);

    // --- Init CS for Balanserer (PB10) ---
    GPIO_InitStruct.Pin = CS_Balanserer_PIN;
    HAL_GPIO_Init(CS_Balanserer_GPIO_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(CS_Balanserer_GPIO_PORT, CS_Balanserer_PIN, GPIO_PIN_SET);

    // --- Init CS for Balanserer (PB10) ---
    GPIO_InitStruct.Pin = CS_TempIC_PIN;
    HAL_GPIO_Init(CS_TempIC_GPIO_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(CS_TempIC_GPIO_PORT, CS_TempIC_PIN, GPIO_PIN_SET);

}

void CS_Overvoker_Enable(void) {
    HAL_GPIO_WritePin(CS_Overvoker_GPIO_PORT, CS_Overvoker_PIN, GPIO_PIN_RESET);
}

void CS_Overvoker_Disable(void) {
    HAL_GPIO_WritePin(CS_Overvoker_GPIO_PORT, CS_Overvoker_PIN, GPIO_PIN_SET);
}
void CS_Balanserer_Enable(void) {
    HAL_GPIO_WritePin(CS_Balanserer_GPIO_PORT, CS_Balanserer_PIN, GPIO_PIN_RESET);
}

void CS_Balanserer_Disable(void) {
    HAL_GPIO_WritePin(CS_Balanserer_GPIO_PORT, CS_Balanserer_PIN, GPIO_PIN_SET);
}
void CS_TempIC_Enable(void) {
	HAL_GPIO_WritePin(CS_TempIC_GPIO_PORT, CS_TempIC_PIN, GPIO_PIN_RESET);
}
void CS_TempIC_Disable(void) {
	HAL_GPIO_WritePin(CS_TempIC_GPIO_PORT, CS_TempIC_PIN, GPIO_PIN_SET);
}


// Send and receive simultaneously (full-duplex)
HAL_StatusTypeDef SPI_SendReceive(uint8_t *txData, uint8_t *rxData, uint16_t size) {
    return HAL_SPI_TransmitReceive(&SPI_HANDLE, txData, rxData, size, HAL_MAX_DELAY);
}

// Send only
HAL_StatusTypeDef SPI_Send(uint8_t *txData, uint16_t size) {
    return HAL_SPI_Transmit(&SPI_HANDLE, txData, size, HAL_MAX_DELAY);
}

// Receive only
HAL_StatusTypeDef SPI_Receive(uint8_t *rxData, uint16_t size) {
    return HAL_SPI_Receive(&SPI_HANDLE, rxData, size, HAL_MAX_DELAY);
}

















