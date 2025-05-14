/*
 * balanserer.c
 *
 *  Created on: May 6, 2025
 *      Author: Rolfk
 */


#include "balanserer.h"
#include <stdint.h>
#include <stdlib.h>  // for qsort()
#include <stdbool.h>
#include <stdio.h>

uint8_t Write_Balance_Command = 0xA9;
uint8_t Read_Balance_Command = 0xAA;
uint8_t Read_Balance_Status_Command = 0xAC;
uint8_t Execute_Balance_Command = 0xAF;

uint8_t stop_cell_actions[6] = {0, 0, 0, 0, 0, 0};  // all no action
uint8_t stop_config_bytes[2];

#define BALANCE_DISCHARGE 2  // 10
#define BALANCE_CHARGE    3  // 11
#define BALANCE_NONE      0  // 00

#define BALANCE_THRESHOLD 0.1f  // tolerance (volts) around median

uint8_t calc_crc4(uint16_t message) {
    uint16_t poly = 0x13; // x^4 + x + 1
    message <<= 4;
    for (int i = 0; i < 12; i++) {
        if (message & 0x8000) {
            message ^= (poly << 11);
        }
        message <<= 1;
    }
    return (message >> 12) & 0xF;
}

uint16_t build_balance_command(uint8_t cell_actions[6]) {
    uint16_t command = 0;
    for (int i = 0; i < 6; i++) {
        command <<= 2;
        command |= (cell_actions[i] & 0x3);
    }
    return command;
}

void make_config_bytes(uint8_t cell_actions[6], uint8_t config_bytes[2]) {
    uint16_t command = build_balance_command(cell_actions);
    uint8_t crc = calc_crc4(command);
    crc = ~crc & 0xF; // invert
    uint16_t config = (command << 4) | crc;

    config_bytes[0] = (config >> 8) & 0xFF; // MSB first
    config_bytes[1] = config & 0xFF;        // LSB
}

// comparison function for qsort
int compare_floats(const void* a, const void* b) {
    float fa = *(const float*)a;
    float fb = *(const float*)b;
    return (fa > fb) - (fa < fb);
}

void determine_cell_actions(const float *voltages, uint8_t *cell_actions) {
    float sorted_voltages[12];
    for (int i = 0; i < 12; i++) {
        sorted_voltages[i] = voltages[i];
    }

    qsort(sorted_voltages, 12, sizeof(float), compare_floats);

    float median = (sorted_voltages[5] + sorted_voltages[6]) / 2.0f;  // median of 12 cells

    for (int i = 0; i < 6; i++) {  // check first 6 cells only
        if (voltages[i] > median + BALANCE_THRESHOLD) {
            cell_actions[i] = BALANCE_DISCHARGE;
        } else if (voltages[i] < median - BALANCE_THRESHOLD) {
            cell_actions[i] = BALANCE_CHARGE;
        } else {
            cell_actions[i] = BALANCE_NONE;
        }
    }
}

void Balanserer_WriteConfig(uint8_t config_bytes[2]) {
    CS_Balanserer_Enable();
    SPI_Send(&Write_Balance_Command, 1);
    SPI_Send(config_bytes, 2);
    CS_Balanserer_Disable();
    printf("Config written: 0x%02X 0x%02X\r\n", config_bytes[0], config_bytes[1]);
}

void Balanserer_ReadConfig(uint8_t *config_buffer) {
    CS_Balanserer_Enable();
    SPI_Send(&Read_Balance_Command, 1);
    SPI_Receive(config_buffer, 2);
    CS_Balanserer_Disable();
}

void Balanserer_ReadStatus(uint8_t *status_buffer) {
    CS_Balanserer_Enable();
    SPI_Send(&Read_Balance_Status_Command, 1);
    SPI_Receive(status_buffer, 2);
    CS_Balanserer_Disable();
}

bool Balanserer_VerifyConfig(uint8_t expected_config[2]) {
    uint8_t command = 0xAA;  // Read Balance Command
    uint8_t rx_buffer[2];

    CS_Balanserer_Enable();
    SPI_Send(&command, 1);
    SPI_Receive(rx_buffer, 2);
    CS_Balanserer_Disable();

    if (rx_buffer[0] == expected_config[0] && rx_buffer[1] == expected_config[1]) {
        return true;  // verified
    } else {
        printf("Verification failed: expected 0x%02X 0x%02X, got 0x%02X 0x%02X\r\n",
               expected_config[0], expected_config[1],
               rx_buffer[0], rx_buffer[1]);
        return false;
    }
}

void Balanserer_ExecuteBalancing(void) {
    CS_Balanserer_Enable();
    SPI_Send(&Execute_Balance_Command, 1);
    CS_Balanserer_Disable();
    printf("Balancing Executed\r\n");
}

void Balanserer_StopBalancing(void) {
    // Generate zero config with valid CRC
    make_config_bytes(stop_cell_actions, stop_config_bytes);

    // Write zero config using existing function
    Balanserer_WriteConfig(stop_config_bytes);

    // Execute the zero config to apply stop
    Balanserer_ExecuteBalancing();

    printf("Balancing stopped (zero config written and executed)\r\n");
}

