/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "spi.h"
#include "uart.h"
//#include "can_bus.h"
#include "overvoker.h"
#include "balanserer.h"
#include "functions.h"
#include "ina238.h"
#include <stdbool.h>


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

COM_InitTypeDef BspCOMInit;

FDCAN_HandleTypeDef hfdcan1;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */
//#define BATTERY_CAPACITY_AH_TOTAL 19.2f
#define BATTERY_CAPACITY_AH 14.8f
#define FULL_VOLTAGE_THRESHOLD 50.0f
#define INA238_SHUNT_CAL 0x100A  // or use your tuned value
// Update rate (s) — adjust if not 1Hz
#define SOC_SAMPLE_INTERVAL_SEC 1.0f
// Tracks total discharge since startup
static float accumulated_ah = 0.0f;

float temperatures[3];
uint8_t celler_balansestatus[12];

uint16_t TX_Buffer = 0;
uint16_t RX_Buffer = 0;
uint8_t Write_command = 0xA9;
uint8_t Read_command = 0xAA;

uint8_t Write_config[2] = {0x30, 0x86};
uint8_t Write_config_2[2] = {0x03, 0x35};

uint8_t Read_config[2];
uint8_t cell_actions[6];// = {0, 0, 0, 3, 0, 2}; // example: no action, discharge, no action, charge, no action, discharge(sync)
uint8_t config_bytes[2];
uint8_t Read_buffer[2];

uint8_t teller = 0;

float dummy_voltages[12] = {
    3.855f,  // Cell 1 → above median → should discharge
    3.855f,  // Cell 2 → above median → should discharge
    3.965f,  // Cell 3 → below median → no action
    3.855f,  // Cell 4 → below median → charge
    3.855f,  // Cell 5 → close to median → no action
    3.855f,  // Cell 6 → below median → charge
    3.855f, // Cell 7 → equal to median → no action
    3.855f, // Cell 8 → equal to median → no action
    3.855f, // Cell 9 → equal to median → no action
    3.855f, // Cell 10 → equal to median → no action
    3.855f, // Cell 11 → equal to median → no action
    3.855f  // Cell 12 → equal to median → no action
};


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_FDCAN1_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void PackBatteryStatusCAN(
    float voltage,         // V (e.g. 40.00–51.00)
    float current,         // A (0.00–30.00)
    uint16_t power,        // W (0–1600)
    float temperature,     // °C (10–80)
    uint8_t soc_percent,   // 0–100%
    bool unbalanced,
    bool vann,
    bool overtemp,
    uint8_t out[8]
) {
    if (voltage > 51.00f) voltage = 51.00f;
    if (voltage < 40.00f) voltage = 40.00f;
    if (current > 30.0f) current = 30.0f;
    if (temperature > 127.0f) temperature = 127.0f;
    if (soc_percent > 100) soc_percent = 100;

    uint16_t v_enc = (uint16_t)(voltage * 100.0f);
    uint8_t  i_enc = (uint8_t)(current * 10.0f);
    uint8_t  t_enc = (uint8_t)(temperature);
    uint8_t  charge = soc_percent;

    uint8_t t_flags = (t_enc & 0x1F);  // use only lower 5 bits for temp
    if (unbalanced) t_flags |= (1 << 7);
    if (vann)       t_flags |= (1 << 6);
    if (overtemp)   t_flags |= (1 << 5);

    out[0] = (v_enc >> 8) & 0xFF;
    out[1] = v_enc & 0xFF;

    out[2] = i_enc;

    out[3] = (power >> 8) & 0xFF;
    out[4] = power & 0xFF;

    out[5] = t_flags;

    out[6] = charge;

    out[7] = 0; // reserved
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_FDCAN1_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  HAL_FDCAN_Start(&hfdcan1);

  SPI_Devices_Init();
  Overvoker_Init();
  if (INA238_Init(&hi2c1, INA238_SHUNT_CAL) == HAL_OK) {
      printf("INA238 initialized.\n");
  } else {
      printf("INA238 init failed.\n");
  }
//  CAN_Init();

//  static bool soc_initialized = false;
//
//  float v_bus = INA238_GetBusVoltage(&hi2c1);
//  if (!soc_initialized && v_bus > FULL_VOLTAGE_THRESHOLD) {
//      // Reset accumulated charge (register 0x0A)
//      if (INA238_WriteRegister(&hi2c1, INA238_REG_CHARGE, 0x2000) == HAL_OK) {
//          printf("Battery initialized as full (%.2f V), charge counter reset.\n", v_bus);
//          soc_initialized = true;
//      } else {
//          printf("Failed to reset INA238 charge register.\n");
//      }
//  }
  /* USER CODE END 2 */

  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
//	  if (HAL_GetTick() - last > 1000) {
//	      printf("Time: %lu ms\r\n", HAL_GetTick());
//	      last = HAL_GetTick();
//	  }

	  bool unbalanced = false;
  	  bool vann = false;
  	  bool overtemp = false;

  	  Vanninntrengning(&vann);

	  Overvoker_writeRegisters();
	  float voltages[12];
	  float deviation[12];
	  int8_t charge_status[12];
	  Overvoker_readVolts(voltages);
	  Overvoker_EvaluateBalance(voltages, deviation, charge_status);


	  for (int i = 0; i < 12; i++) {
	      if (charge_status[i] != 0) {
	          unbalanced = true;
	          break;
	      }
	      else {
	    	  unbalanced = false;
	      }
	  }

	  // Print Voltage for cells and deviance and status
//	  for (int i = 0; i < 12; i++) {
//	      printf("Cell %d: %.3f V, Δ=%.3f, status=%d\r\n", i + 1, voltages[i], deviation[i], charge_status[i]);
//	  }

	  // UART in csv format
	  for (int i = 0; i < 12; i++) {
	      printf("%.3f", voltages[i]);
	      if (i < 11) printf(",");
	  }
	  printf(",");
//	  printf("\r\n");

	  // Les temperaturer
	  Overvoker_readTemp(temperatures);

	      // Skriv ut temperaturene
//	  printf("Temp 1 (ext): %.2f °C\r\n", temperatures[0]);
//	  printf("Temp 2 (ext): %.2f °C\r\n", temperatures[1]);
//	  printf("Temp IC (int): %.2f °C\r\n", temperatures[2]);

	  // Find the highest temperature
	  float max_temp = temperatures[0];
	  for (int i = 1; i < 3; i++) {
	      if (temperatures[i] > max_temp) {
	          max_temp = temperatures[i];
	      }
	  }
	  CheckOvertemp(max_temp, &overtemp);
	  // Then 3 temperatures
	  for (int i = 0; i < 3; i++) {
	      printf("%.2f,", temperatures[i]);
	  }
//	  printf("Highest temp: %.2f °C\r\n", max_temp);

	  static uint32_t last_tick = 0;
	  uint32_t now = HAL_GetTick();  // in ms
	  float delta_sec = (now - last_tick) / 1000.0f;
	  last_tick = now;

	  // Only integrate if delta is valid (e.g. >0.001 s)
	  if (delta_sec > 0.001f) {
	      float current = INA238_GetCurrent(&hi2c1);
	      if (current < 0.0f) current = 0.0f;

	      accumulated_ah += (current * delta_sec) / 3600.0f;

	      if (accumulated_ah > BATTERY_CAPACITY_AH)
	          accumulated_ah = BATTERY_CAPACITY_AH;

	      float soc = 100.0f * (1.0f - (accumulated_ah / BATTERY_CAPACITY_AH));
	      if (soc < 0.0f) soc = 0.0f;

	      uint8_t soc_percent = (uint8_t)(soc + 0.5f);

	      printf("%.3f,", current);
//	      printf("Acc. Discharge: %.6f Ah\r\n", accumulated_ah);
//	      printf("SoC: %d %%\r\n", soc_percent);


	      // Add discharge (Ah), SoC (%), Bus Voltage (V), Power (W)
	      printf("%.6f,", accumulated_ah);                     // Discharge
	      printf("%d,", soc_percent);                          // SoC
	      printf("%.3f,", INA238_GetBusVoltage(&hi2c1));       // Vbus
	      printf("%.3f\r\n", INA238_GetPower(&hi2c1));         // Power

	      uint8_t can_data[8];
	      PackBatteryStatusCAN(
	    		  INA238_GetBusVoltage(&hi2c1),
				  INA238_GetCurrent(&hi2c1),
				  INA238_GetPower(&hi2c1),
				  max_temp,  // max temp
	      	      soc_percent,               // % SoC
	      	      unbalanced,             // unbalanced
				  vann,
				  overtemp,
	      	      can_data
				  );
//	      printf("CAN Data: ");
//	      for (int i = 0; i < 8; i++) {
//	          printf("%02X ", can_data[i]);
//	      }
	      printf("\r\n");






	      FDCAN_TxHeaderTypeDef txHeader;
	      txHeader.Identifier = 0x84;
	      txHeader.IdType = FDCAN_STANDARD_ID;
	      txHeader.TxFrameType = FDCAN_DATA_FRAME;
	      txHeader.DataLength = FDCAN_DLC_BYTES_8;  // 8 bytes of data
	      txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	      txHeader.BitRateSwitch = FDCAN_BRS_OFF;
	      txHeader.FDFormat = FDCAN_CLASSIC_CAN;
	      txHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	      txHeader.MessageMarker = 0;

//	      if (teller >= 10) {
	      if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &txHeader, can_data) != HAL_OK) {
	              printf("HAL_FDCAN transmission failed!\r\n");
//	    		  teller = 0;  // Reset the counter after sending
	      }
//	      }
//	      else {
//	    	  teller++;
//	      }
	  }
//	  printf("Bus Voltage: %.3f V\r\n", INA238_GetBusVoltage(&hi2c1));
////	  printf("Shunt Voltage: %.6f V\r\n", INA238_GetShuntVoltage(&hi2c1));
////	  printf("Current: %.3f A\r\n", INA238_GetCurrent(&hi2c1));
//	  printf("Power: %.3f W\r\n", INA238_GetPower(&hi2c1));







//
//	  CAN_Send(0x123, can_data, 8);


	  Wait(1000);


//	  determine_cell_actions(dummy_voltages, cell_actions);
//	  make_config_bytes(cell_actions, config_bytes);
//	  Balanserer_WriteConfig(config_bytes);

//	  if (Balanserer_VerifyConfig(config_bytes)) {
//	      Balanserer_ExecuteBalancing();
//		  printf("Config verified! Balancing executed.\r\n");
//		  Balanserer_ExecuteBalancing();
//		  Balanserer_ReadStatus(Read_buffer);
//		  printf("Read: 0x%02X 0x%02X\r\n", Read_buffer[0], Read_buffer[1]);
//		  Wait(2000);
//	      Balanserer_StopBalancing();
//	      printf("Balancing Stopped.\r\n");
//	  } else {
//		  Balanserer_StopBalancing();
//	      printf("Config verification failed! Balancing aborted.\r\n");
//	  }
//
////	  printf("Received bytes: 0x%02X 0x%02X\n", config_bytes[0], config_bytes[1]);
//	  Wait(3000);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief FDCAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = ENABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 20;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 14;
  hfdcan1.Init.NominalTimeSeg2 = 2;
  hfdcan1.Init.DataPrescaler = 20;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 14;
  hfdcan1.Init.DataTimeSeg2 = 2;
  hfdcan1.Init.StdFiltersNbr = 1;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x40B285C2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9|GPIO_PIN_10, GPIO_PIN_RESET);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA9 PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB4 PB5 PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
