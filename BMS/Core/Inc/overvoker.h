/*
 * overvoker.h
 *
 *  Created on: Apr 30, 2025
 *      Author: rolf
 */

#ifndef INC_OVERVOKER_H_
#define INC_OVERVOKER_H_

#include <stdint.h>

//#define WRCFG 0x01             	// Write Config. Registers
//#define WRCFG_PEC 0xC7			// The PEC =0xC7
//#define STCVDC 0x60   			// A/D converter and poll status command
//#define STCVDC_PEC 0xE7			// The PEC =0xE7
//#define RDCV 0x04              	// Read cell voltages command
//#define RDCV_PEC 0xDC			// The PEC =0xDC
//#define STTMPAD 0x30           	// Start All Temp. ADC and Poll Status
//#define STTMPAD_PEC 0x50		// The PEC =0x50
//#define RDTMP 0x0E             	// Read Temp registers group
//#define RDTMP_PEC 0xEA			// The PEC = 0xEA

void Overvoker_Init(void);

void Overvoker_findRegisters(void);
uint8_t Overvoker_getPEC(void);
void Overvoker_writeRegisters(void);
// Function to read raw voltage bytes
void Overvoker_readVolts(float *cell_voltages);
void Overvoker_EvaluateBalance(float voltages[12], float diff[12], int8_t status[12]);
void Overvoker_readTemp(float *temps);

#endif /* INC_OVERVOKER_H_ */
