/*
 * functions.h
 *
 *  Created on: Apr 30, 2025
 *      Author: rolf
 */

#ifndef INC_FUNCTIONS_H_
#define INC_FUNCTIONS_H_

#include <stdint.h>       // for uint32_t
#include "stm32g4xx_hal.h"  // for HAL_Delay()
#include <stdbool.h>

// Wait function (in milliseconds)
void Wait(uint32_t ms);
// Vannvarsler pinne
#define VANN_PORT			GPIOA
#define VANN				GPIO_PIN_8

void Vanninntrengning(bool *vann);
void CheckOvertemp(float temperature, bool *overtemp);

#endif /* INC_FUNCTIONS_H_ */
