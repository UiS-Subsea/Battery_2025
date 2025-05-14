/*
 * overvoker.c
 *
 *  Created on: Apr 30, 2025
 *      Author: rolf
 */
#include "spi.h"
#include "main.h"
#include "functions.h"
#include "overvoker.h"
#include <math.h>
#include <stdbool.h>
#include <string.h>  // for memcpy
#include <stdlib.h>  // for qsort


uint8_t WRCFG = 0x01;			// Write Config. Registers
uint8_t WRCFG_PEC = 0xC7;		// The PEC =0xC7

uint8_t STCVDC = 0x60;			// A/D converter and poll status command
uint8_t STCVDC_PEC = 0xE7;		// The PEC =0xE7

uint8_t RDCV = 0x04;            // Read cell voltages command
uint8_t RDCV_PEC = 0xDC;		// The PEC =0xDC

uint8_t STTMPAD = 0x30;         // Start All Temp. ADC and Poll Status
uint8_t STTMPAD_PEC = 0x50;		// The PEC =0x50

uint8_t RDTMP = 0x0E;           // Read Temp registers group
uint8_t RDTMP_PEC =	0xEA;		// The PEC = 0xEA

uint8_t WRCFGval[6];			// valriable to hold all 6 bytes of config registers values
uint8_t cvr[18];				// cvr to store the 8bit value read from LTC

uint8_t _cellNumber = 12;		// Number of battery cells - maximum is 12 cells
float cell_voltages[12];
float _cell_voltages[12];		// cell_voltage[x] contains the actual 12bit measured voltage value for Cell number "x"

uint8_t tmp[6];					// tmp to store the 8bit value read from LTC
uint16_t _tmp_cell[12];			// _tmp_cell[x] contains the actual 12bit measured temperature value in Degrees Celsuis
float _tmp_val;

float resist[2];         		// calculated resistance
float rinf = 0;              		// reference resistance
float IC_tmp;            		// final temp result (°C)
float tmp_cell[2];       		// public values

float B = 3950.0f;

float temps[3];

/* Setting the variables for configuration registers,
this will make it easier and faster to cange parameters in the configuration registers*/
uint8_t VOV = 0;           // Overvoltage threshold configuration byte
uint8_t VUV = 0;           // Undervoltage threshold configuration byte
float offset = 0.0015f;     // ADC quantization (V/Bit), e.g., 1.5mV per bit
float calibration_factor = 0.84429f;  // eksempel: mål faktisk spenning og finn faktor
float calibration_factor_tmpi = 0.781f;
float calibration_factor_tmpe = 0.69f;

uint8_t CELL10 = 0;        // 1 if less than 12 cells; 0 otherwise
uint8_t WDT = 1;           // Watchdog timer bit; 1 = enabled (default)
uint8_t LVLPL = 0;         // Polling/toggle mode bit; 0 = toggle (default)
uint8_t CDC = 1;           // Comparator duty cycle; 0..7 (last 3 bits only)
uint8_t GPI01 = 0;         // General purpose pin 1 bit; 1 = enabled (default)
uint8_t GPI02 = 0;         // General purpose pin 2 bit; 1 = enabled (default)

uint8_t DCC[12] = {0};     // Discharge control bits for 12 cells; initialized to 0 (no discharge)

uint8_t MC1  = 0;          // Mask cell 1 in config register
uint8_t MC2  = 0;          // Mask cell 2
uint8_t MC3  = 0;          // Mask cell 3
uint8_t MC4  = 0;          // Mask cell 4
uint8_t MC5  = 0;          // Mask cell 5
uint8_t MC6  = 0;          // Mask cell 6
uint8_t MC7  = 0;          // Mask cell 7
uint8_t MC8  = 0;          // Mask cell 8
uint8_t MC9  = 0;          // Mask cell 9
uint8_t MC10 = 0;          // Mask cell 10
uint8_t MC11 = 0;          // Mask cell 11
uint8_t MC12 = 0;          // Mask cell 12

uint8_t pec1 = 0;  // store PEC byte (8-bit) & The calculated PEC value using the function



void Overvoker_Init(void) {
	Overvoker_findRegisters();
	Overvoker_getPEC();
	Overvoker_writeRegisters();
}

void Overvoker_findRegisters(void) {
    WRCFGval[0] = (WDT << 7) | (GPI02 << 6) | (GPI01 << 5) | (LVLPL << 4) | (CELL10 << 3) | (CDC & 0x07);
    WRCFGval[1] = (DCC[7]<<7)|(DCC[6]<<6)|(DCC[5]<<5)|(DCC[4]<<4)|(DCC[3]<<3)|(DCC[2]<<2)|(DCC[1]<<1)|DCC[0];
    WRCFGval[2] = (MC4<<7)|(MC3<<6)|(MC2<<5)|(MC1<<4)|(DCC[11]<<3)|(DCC[10]<<2)|(DCC[9]<<1)|(DCC[8]);
    WRCFGval[3] = (MC12 << 7) | (MC11 << 6) | (MC10 << 5) | (MC9 << 4) | (MC8 << 3) | (MC7 << 2) | (MC6 << 1) | MC5;
    WRCFGval[4] = 0;  // VUV
    WRCFGval[5] = 0;  // VOV
}

uint8_t Overvoker_getPEC(void) {
    uint8_t pec = 0x41;
    uint8_t in0, in1, in2;
    int j, i;

    for (j = 0; j < 6; j++) {
        for (i = 0; i < 8; i++) {
            in0 = ((WRCFGval[j] >> (7 - i)) & 0x01) ^ ((pec >> 7) & 0x01);
            in1 = in0 ^ ((pec >> 0) & 0x01);
            in2 = in0 ^ ((pec >> 1) & 0x01);
            pec = in0 | (in1 << 1) | (in2 << 2) | ((pec << 1) & ~0x07);
        }
    }
    return pec;
}

void Overvoker_writeRegisters(void) {
    uint8_t pec = Overvoker_getPEC();
    uint8_t tx_data[9];

    tx_data[0] = WRCFG;       // command
    tx_data[1] = 0xC7;        // command PEC
    for (int i = 0; i < 6; i++) {
        tx_data[2 + i] = WRCFGval[i];  // config bytes
    }
    tx_data[8] = pec;         // config PEC

    CS_Overvoker_Enable();  // pull CS low

    SPI_Send(tx_data, 9);
//    HAL_SPI_Transmit(&hspi1, tx_data, 9, HAL_MAX_DELAY);


    CS_Overvoker_Disable();   // pull CS high
    Wait(1);             // 1ms delay
}

void Overvoker_readVolts(float *cell_voltages)
{


    // --- Start ADC Conversion ---

    CS_Overvoker_Enable(); 		// Selecting the chip
    SPI_Send(&STCVDC, 1); 		//   Send start ADC conversion command
    SPI_Send(&STCVDC_PEC, 1); 	//   Send start ADC conversion command PEC
    CS_Overvoker_Disable(); 	// De-selecting the chip

    Wait(15);  					// Wait for conversion in ms
    // --- Read Converted Voltages ---
    CS_Overvoker_Enable(); 		// Selecting the chip
    SPI_Send(&RDCV, 1);			// Reading all Cell Voltage Group command
    SPI_Send(&RDCV_PEC, 1);		// RDCV PEC


    //Dummy DATA
//    for (int i = 0; i < 18; i++) {
//    	cvr[i] = 0xFF;}  // arbitrary test pattern

    SPI_Receive(cvr, 18);	//Store 18 bytes in cvr_out

    CS_Overvoker_Disable(); 	// De-selecting the chip

    // --- Reconstruct 12-bit Cell Voltages ---
    _cell_voltages[0]  = (cvr[0]  & 0xFF) | (cvr[1] & 0x0F) << 8;
    _cell_voltages[1]  = (cvr[1]  & 0xF0) >> 4 | (cvr[2] & 0xFF) << 4;
    _cell_voltages[2]  = (cvr[3]  & 0xFF) | (cvr[4] & 0x0F) << 8;
    _cell_voltages[3]  = (cvr[4]  & 0xF0) >> 4 | (cvr[5] & 0xFF) << 4;
    _cell_voltages[4]  = (cvr[6]  & 0xFF) | (cvr[7] & 0x0F) << 8;
    _cell_voltages[5]  = (cvr[7]  & 0xF0) >> 4 | (cvr[8] & 0xFF) << 4;
    _cell_voltages[6]  = (cvr[9]  & 0xFF) | (cvr[10] & 0x0F) << 8;
    _cell_voltages[7]  = (cvr[10] & 0xF0) >> 4 | (cvr[11] & 0xFF) << 4;
    _cell_voltages[8]  = (cvr[12] & 0xFF) | (cvr[13] & 0x0F) << 8;
    _cell_voltages[9]  = (cvr[13] & 0xF0) >> 4 | (cvr[14] & 0xFF) << 4;
    _cell_voltages[10] = (cvr[15] & 0xFF) | (cvr[16] & 0x0F) << 8;
    _cell_voltages[11] = (cvr[16] & 0xF0) >> 4 | (cvr[17] & 0xFF) << 4;

    // --- Convert to Voltage Values ---
    for (int i = 0; i < _cellNumber; i++) {
        cell_voltages[i] = _cell_voltages[i] * offset * calibration_factor;
    }
}

static int compare_floats(const void *a, const void *b) {
    float fa = *(const float*)a;
    float fb = *(const float*)b;
    return (fa > fb) - (fa < fb);
}

void Overvoker_EvaluateBalance(float voltages[12], float diff[12], int8_t status[12]) {
    float sorted[12];
    memcpy(sorted, voltages, sizeof(float) * 12);

    // Sort to find median
    qsort(sorted, 12, sizeof(float), compare_floats);
    float median = (sorted[5] + sorted[6]) / 2.0f;  // median of 12 values

    float threshold = 0.1f;  // 100 mV threshold

    for (int i = 0; i < 12; i++) {
        diff[i] = voltages[i] - median;

        if (diff[i] > threshold) {
            status[i] = 1;  // overcharged
        } else if (diff[i] < -threshold) {
            status[i] = -1; // undercharged
        } else {
            status[i] = 0;  // in balance
        }
    }
}

void Overvoker_readTemp(float *temps)
{
	// First start ADC conversion
	CS_Overvoker_Enable(); 		// Selecting the chip
	SPI_Send(&STTMPAD, 1); 		// Send start all Temp. ADC conversion command
	SPI_Send(&STTMPAD_PEC, 1); 	// Send start ADC conversion command PEC
	CS_Overvoker_Disable(); 	// De-selecting the chip

	Wait(10);					// Wait for ADC conversion

	// Then read ADC results
	CS_Overvoker_Enable();		// Selecting the chip
	SPI_Send(&RDTMP, 1);			// Read all Temp reg. Group command
	SPI_Send(&RDTMP_PEC, 1);		// RDTMP PEC
	SPI_Receive(tmp, 5);		// Storing each received byte in a different variable
	CS_Overvoker_Disable();		// De-selecting the chip

	Wait(10);

	// --- Rebuild 12-bit values ---
	_tmp_cell[0] = (tmp[0]) | ((tmp[1] & 0x0F) << 8);
	_tmp_cell[1] = ((tmp[1] >> 4) & 0x0F) | (tmp[2] << 4);
	_tmp_val     = (tmp[3]) | ((tmp[4] & 0x0F) << 8);

	// --- Convert to resistance from voltage ---
	float v0 = _tmp_cell[0] * offset * calibration_factor_tmpe;
	float v1 = _tmp_cell[1] * offset * calibration_factor_tmpe;
	float vi = _tmp_val * offset * calibration_factor_tmpi;

	float Vcc = 3.02f;
	float Rref = 10000.0f;

	resist[0] = (v0 * Rref) / (Vcc - v0);
	resist[1] = (v1 * Rref) / (Vcc - v1);

	// Bruk standard Steinhart-Hart
    float R0 = 10000.0f;  // 10k NTC
    float T0 = 298.15f;   // 25°C i Kelvin

    temps[0] = 1.0f / ( (1.0f / T0) + (1.0f / B) * logf(resist[0] / R0) ) - 273.15f;
    temps[1] = 1.0f / ( (1.0f / T0) + (1.0f / B) * logf(resist[1] / R0) ) - 273.15f;
    // Intern LTC6803 temp
    temps[2] = (vi / 0.00839f) - 273.15f;


}










