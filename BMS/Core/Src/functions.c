/*
 * functions.c
 *
 *  Created on: Apr 30, 2025
 *      Author: rolf
 */

//#include <stdint.h>
#include "functions.h"

void Wait(uint32_t ms) {
    HAL_Delay(ms);
}

#include <stdbool.h>
#include <stdio.h>


//bool vann = false;

void Vanninntrengning(bool *vann) {
    if (HAL_GPIO_ReadPin(VANN_PORT, VANN) == GPIO_PIN_RESET) {
        *vann = true;  // lav → inntrengning
        printf("Vann");
    } else {
        *vann = false; // høy → ingen inntrengning
    }
}

void CheckOvertemp(float temperature, bool *overtemp) {
    if (temperature > 60.0f) {
        *overtemp = true;
        printf("Overtemperature detected: %.2f °C\n", temperature);
    } else {
        *overtemp = false;
    }
}
