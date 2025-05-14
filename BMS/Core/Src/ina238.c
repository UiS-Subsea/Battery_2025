/*
 * ina238.c
 *
 *  Created on: May 6, 2025
 *      Author: Rolfk
 */


#include "ina238.h"

HAL_StatusTypeDef INA238_ReadRegister(I2C_HandleTypeDef *hi2c, uint8_t reg, uint16_t* value) {
    uint8_t buf[2];
    HAL_StatusTypeDef ret;

    ret = HAL_I2C_Mem_Read(hi2c, INA238_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
    if (ret == HAL_OK) {
        *value = (buf[0] << 8) | buf[1];  // big-endian
    }
    return ret;
}

HAL_StatusTypeDef INA238_WriteRegister(I2C_HandleTypeDef *hi2c, uint8_t reg, uint16_t value) {
    uint8_t buf[2];
    buf[0] = (value >> 8) & 0xFF;
    buf[1] = value & 0xFF;
    return HAL_I2C_Mem_Write(hi2c, INA238_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY);
}

HAL_StatusTypeDef INA238_Init(I2C_HandleTypeDef *hi2c, uint16_t shunt_cal_value) {
    // Only write SHUNT_CAL — no charge accumulation possible
    return INA238_WriteRegister(hi2c, INA238_REG_SHUNT_CAL, shunt_cal_value);

}

float INA238_GetBusVoltage(I2C_HandleTypeDef *hi2c) {
    uint16_t raw;
    if (INA238_ReadRegister(hi2c, INA238_REG_BUS_VOLTAGE, &raw) != HAL_OK) return -1.0f;
    return raw * 3.125f / 1000.0f;  // LSB = 3.125mV → V
}

float INA238_GetShuntVoltage(I2C_HandleTypeDef *hi2c) {
    uint16_t raw;
    if (INA238_ReadRegister(hi2c, INA238_REG_SHUNT_VOLTAGE, &raw) != HAL_OK) return -1.0f;
    return raw * 1.25f / 1e6f;  // LSB = 1.25uV → V
}

float INA238_GetCurrent(I2C_HandleTypeDef *hi2c) {
    uint16_t raw;
    if (INA238_ReadRegister(hi2c, INA238_REG_CURRENT, &raw) != HAL_OK) return -1.0f;
    int16_t signed_raw = (int16_t)raw;
    return signed_raw * 1.25f / 1000.0f;  // LSB = 1.25mA (depends on SHUNT_CAL)
}

float INA238_GetPower(I2C_HandleTypeDef *hi2c) {
    uint16_t raw;
    if (INA238_ReadRegister(hi2c, INA238_REG_POWER, &raw) != HAL_OK) return -1.0f;
    return raw * (25.0f * 0.0025326f);  // W
    // = raw * 0.0244f

}
