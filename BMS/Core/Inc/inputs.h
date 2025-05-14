/*
 * inputs.h
 *
 *  Created on: Apr 30, 2025
 *      Author: rolf
 */

#ifndef INC_INPUTS_H_
#define INC_INPUTS_H_


#include "stm32g4xx_hal.h"

// Input Pin Definitions
#define Vann_GPIO_PORT    	GPIOA
#define Vann_PIN          	GPIO_PIN_8
#define AlertIO_GPIO_PORT 	GPIOB
#define AlertIO_PIN			GPIO_PIN_4
#define OverTMP_GPIO_PORT	GPIOB
#define OverTMP_PIN			GPIO_PIN_5
#define CritTMP_GPIO_PORT	GPIOB
#define CritTMP_PIN			GPIO_PIN_6

// Function Prototypes
void Inputs_Init(void);
uint8_t Power_meas_Alert(void);

#endif /* INC_INPUTS_H_ */
