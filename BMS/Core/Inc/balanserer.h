/*
 * balanserer.h
 *
 *  Created on: May 6, 2025
 *      Author: Rolfk
 */

#ifndef INC_BALANSERER_H_
#define INC_BALANSERER_H_

#include "spi.h"  // your existing SPI abstraction
#include <stdint.h>
#include <stdbool.h>

//uint8_t Write_Balance_Command = 0xA9;
//uint8_t Read_Balance_Command = 0xAA;
//uint8_t Read_Balance_Status_Command = 0xAC;
//uint8_t Execute_Balance_Command = 0xAF;

uint8_t calc_crc4(uint16_t message);
uint16_t build_balance_command(uint8_t cell_actions[6]);
void make_config_bytes(uint8_t cell_actions[6], uint8_t config_bytes[2]);
void make_config_bytes(uint8_t cell_actions[6], uint8_t config_bytes[2]);
void determine_cell_actions(const float *voltages, uint8_t *cell_actions);
void Balanserer_WriteConfig(uint8_t config_bytes[2]);
void Balanserer_ReadConfig(uint8_t *config_buffer);
void Balanserer_ReadStatus(uint8_t *status_buffer);
bool Balanserer_VerifyConfig(uint8_t expected_config[2]);
void Balanserer_ExecuteBalancing(void);
void Balanserer_StopBalancing(void);

#endif /* INC_BALANSERER_H_ */
