/*
 * ina238.h
 *
 *  Created on: May 6, 2025
 *      Author: Rolfk
 */

#ifndef INC_INA238_H_
#define INC_INA238_H_

#include "stm32g4xx_hal.h"

// I2C default address (ADDR pin = GND)
#define INA238_I2C_ADDR   (0x40 << 1)  // STM32 HAL uses 8-bit addr

// Register addresses (confirmed with datasheet)
#define INA238_REG_CONFIG            0x00
#define INA238_REG_ADC_CONFIG        0x01
#define INA238_REG_SHUNT_CAL         0x02
#define INA238_REG_SHUNT_VOLTAGE     0x04
#define INA238_REG_BUS_VOLTAGE       0x05
#define INA238_REG_DIETEMP           0x06
#define INA238_REG_CURRENT           0x07
#define INA238_REG_POWER             0x08
#define INA238_REG_MANUFACTURER_ID   0x3E
#define INA238_REG_DEVICE_ID         0x3F

// Function prototypes
HAL_StatusTypeDef INA238_ReadRegister(I2C_HandleTypeDef *hi2c, uint8_t reg, uint16_t* value);
HAL_StatusTypeDef INA238_WriteRegister(I2C_HandleTypeDef *hi2c, uint8_t reg, uint16_t value);
HAL_StatusTypeDef INA238_Init(I2C_HandleTypeDef *hi2c, uint16_t shunt_cal_value);

float INA238_GetBusVoltage(I2C_HandleTypeDef *hi2c);
float INA238_GetShuntVoltage(I2C_HandleTypeDef *hi2c);
float INA238_GetCurrent(I2C_HandleTypeDef *hi2c);
float INA238_GetPower(I2C_HandleTypeDef *hi2c);

#endif /* INC_INA238_H_ */
