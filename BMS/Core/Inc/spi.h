#ifndef INC_SPI_H
#define INC_SPI_H

#include "stm32g4xx_hal.h"

// --- SPI CS Pin Definitions ---
#define CS_Overvoker_GPIO_PORT      GPIOA
#define CS_Overvoker_PIN            GPIO_PIN_9
#define CS_Balanserer_GPIO_PORT     GPIOA
#define CS_Balanserer_PIN           GPIO_PIN_10
#define CS_TempIC_GPIO_PORT			GPIOB
#define CS_TempIC_PIN				GPIO_PIN_0
// Function Prototypes
void SPI_Devices_Init(void);
void CS_Overvoker_Enable(void);
void CS_Overvoker_Disable(void);
void CS_Balanserer_Enable(void);
void CS_Balanserer_Disable(void);
void CS_TempIC_Enable(void);
void CS_TempIC_Disable(void);



// SPI handle (assumes you're using SPI1)
extern SPI_HandleTypeDef hspi1;
#define SPI_HANDLE hspi1

// Init
void SPI_Devices_Init(void);

// SPI I/O
HAL_StatusTypeDef SPI_SendReceive(uint8_t *txData, uint8_t *rxData, uint16_t size);
HAL_StatusTypeDef SPI_Send(uint8_t *txData, uint16_t size);
HAL_StatusTypeDef SPI_Receive(uint8_t *rxData, uint16_t size);













#endif // __SPI_H
